#include "metal_renderer.hpp"

#include "CocoaBridge.hpp"
#include "shader_compiler.hpp"
#include "../node_system/managers/transform/transforms.hpp"

#include <CoreGraphics/CoreGraphics.h>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <cassert>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace bird::node_system::managers;

namespace bird {

struct QuadTransformUniform {
  glm::vec2 local_position;
  glm::vec2 local_scale;
  float local_rotation;
  glm::vec3 padding; // Padding to make the struct size a multiple of 16 bytes for std140 alignment
};

struct MetalRenderer::Impl {
  MTL::Device* metal_device = nullptr;

  CA::MetalLayer* metal_layer = nullptr;
  CA::MetalDrawable* metal_drawable = nullptr;

  MTL::Library* metal_vertex_library = nullptr;
  MTL::Library* metal_fragment_library = nullptr;
  MTL::CommandQueue* metal_command_queue = nullptr;
  MTL::CommandBuffer* metal_command_buffer = nullptr;
  MTL::RenderPipelineState* metal_render_pso = nullptr;
  MTL::Buffer* vertex_buffer = nullptr;
  MTL::Buffer* index_buffer = nullptr;
  MTL::Buffer* transform_buffer = nullptr;
  MTL::SamplerState* sampler_state = nullptr;
};

void MetalRenderer::init_device() {
  if (metal_backend_ == nullptr) {
      metal_backend_ = new Impl();
  }
  metal_backend_->metal_device = MTL::CreateSystemDefaultDevice();
}

void MetalRenderer::init_window(Window& window) {

  void* native_window_handle = window.getNativeHandle();

  int width = window.getFramebufferWidth();
  int height = window.getFramebufferHeight();;

  metal_backend_->metal_layer = CA::MetalLayer::layer();
  metal_backend_->metal_layer->setDevice(metal_backend_->metal_device);
  metal_backend_->metal_layer->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
  metal_backend_->metal_layer->setDrawableSize(CGSizeMake(width, height));

  GLFWBridge::AddLayerToWindow(metal_backend_->glfw_window, metal_backend_->metal_layer);
}


void MetalRenderer::init(Window& window) {
  init_device();
  init_window(window);

  create_default_library();
  create_command_queue();
  create_render_pipeline();
}

void MetalRenderer::render() {
  if (!metal_backend_ || !metal_backend_->metal_layer) {
      return;
  }

  NS::AutoreleasePool* frame_pool = NS::AutoreleasePool::alloc()->init();
  metal_backend_->metal_drawable = metal_backend_->metal_layer->nextDrawable();
  if (metal_backend_->metal_drawable != nullptr) {
      draw();
  }
  frame_pool->release();
}

void MetalRenderer::submit_quad(const RenderCommand render_command) {
  const glm::vec2 local_positions[] = {
      {-0.5f,  0.5f},  // top-left
      { 0.5f,  0.5f},  // top-right
      {-0.5f, -0.5f},  // bottom-left
      { 0.5f, -0.5f}   // bottom-right
  };

  const glm::vec2 uv_coords[] = {
      {0.0f, 1.0f},  // top-left
      {1.0f, 1.0f},  // top-right
      {0.0f, 0.0f},  // bottom-left
      {1.0f, 0.0f}   // bottom-right
  };

  if (!metal_backend_->vertex_buffer) {
    Vertex quad_vertices[4];
    for (int i = 0; i < 4; ++i) {
      quad_vertices[i].x = local_positions[i].x;
      quad_vertices[i].y = local_positions[i].y;
      quad_vertices[i].u = uv_coords[i].x;
      quad_vertices[i].v = uv_coords[i].y;
      quad_vertices[i].color = 0xffffffffu;
      quad_vertices[i].control = 0;
    }

    metal_backend_->vertex_buffer = metal_backend_->metal_device->newBuffer(&quad_vertices, sizeof(quad_vertices), MTL::ResourceStorageModeShared);
  }

  if (!metal_backend_->index_buffer) {
    const std::uint16_t quad_indices[] = {
        0, 1, 2,
        1, 3, 2
    };

    metal_backend_->index_buffer = metal_backend_->metal_device->newBuffer(&quad_indices, sizeof(quad_indices), MTL::ResourceStorageModeShared);
  }

  if (!metal_backend_->transform_buffer) {
    metal_backend_->transform_buffer = metal_backend_->metal_device->newBuffer(sizeof(QuadTransformUniform), MTL::ResourceStorageModeShared);
  }

  const glm::mat3& mat = render_command.transform;

  glm::vec2 position(mat[2]);

  glm::vec2 col0(mat[0]);
  glm::vec2 col1(mat[1]);

  glm::vec2 scale(glm::length(col0), glm::length(col1));

  float determinant = col0.x * col1.y - col0.y * col1.x;
  if (determinant < 0.0f) {
    scale.x = -scale.x;
    col0 = -col0; // Flip the axis back so rotation calculates correctly
  }

  float rotation = std::atan2(col0.y, col0.x);

  QuadTransformUniform quad_transform_uniform {
      position,
      scale,
      rotation,
      glm::vec3(0.0f) // padding for 16 byte alignment
  };

  std::memcpy(metal_backend_->transform_buffer->contents(), &quad_transform_uniform, sizeof(quad_transform_uniform));
  texture = new Texture2D("src/bird_core/rendering/alpha_test.png", metal_backend_->metal_device);
}

void MetalRenderer::shutdown() {
  if (metal_backend_) {
    if (metal_backend_->transform_buffer) {
      metal_backend_->transform_buffer->release();
      metal_backend_->transform_buffer = nullptr;
    }

    if (metal_backend_->vertex_buffer) {
      metal_backend_->vertex_buffer->release();
      metal_backend_->vertex_buffer = nullptr;
    }

    if (metal_backend_->index_buffer) {
      metal_backend_->index_buffer->release();
      metal_backend_->index_buffer = nullptr;
    }
  }

  delete metal_backend_;
  metal_backend_ = nullptr;
}

void MetalRenderer::create_default_library() {
  NS::Error* error = nullptr;

  try {
  ShaderCompiler compiler;

  std::string vert_msl = compiler.compile_glsl_file_to_msl("src/bird_core/rendering/shaders/forward_2d.vert.glsl");
  std::string frag_msl = compiler.compile_glsl_file_to_msl("src/bird_core/rendering/shaders/forward_2d.frag.glsl");

  metal_backend_->metal_vertex_library = metal_backend_->metal_device->newLibrary(
    NS::String::string(vert_msl.c_str(), NS::UTF8StringEncoding),
    nullptr,
    &error
  );

  if (!metal_backend_->metal_vertex_library) {
    std::cerr << "Failed to compile translated vertex shader to Metal";
    if (error != nullptr) {
      std::cerr << "\nMetal error: " << error->localizedDescription()->utf8String();
    }
    std::exit(-1);
  }

  error = nullptr;
  metal_backend_->metal_fragment_library = metal_backend_->metal_device->newLibrary(
    NS::String::string(frag_msl.c_str(), NS::UTF8StringEncoding),
    nullptr,
    &error
  );

  if (!metal_backend_->metal_fragment_library) {
    std::cerr << "Failed to compile translated fragment shader to Metal";
    if (error != nullptr) {
      std::cerr << "\nMetal error: " << error->localizedDescription()->utf8String();
    }
    std::exit(-1);
  }

  MTL::SamplerDescriptor* sampler_descriptor = MTL::SamplerDescriptor::alloc()->init();
  sampler_descriptor->setMinFilter(MTL::SamplerMinMagFilterNearest);
  sampler_descriptor->setMagFilter(MTL::SamplerMinMagFilterNearest);
  sampler_descriptor->setSAddressMode(MTL::SamplerAddressModeClampToEdge);
  sampler_descriptor->setTAddressMode(MTL::SamplerAddressModeClampToEdge);
  metal_backend_->sampler_state = metal_backend_->metal_device->newSamplerState(sampler_descriptor);
  sampler_descriptor->release();

  if (!metal_backend_->sampler_state) {
    std::cerr << "Failed to create Metal sampler state" << std::endl;
    std::exit(-1);
  }
  } catch (const std::exception& e) {
      std::cerr << "Failed to compile shaders: " << e.what() << std::endl;
      std::exit(-1);
  }
}

void MetalRenderer::create_command_queue() {
  metal_backend_->metal_command_queue = metal_backend_->metal_device->newCommandQueue();
}

void MetalRenderer::create_render_pipeline() {
  MTL::Function* vertex_shader = metal_backend_->metal_vertex_library->newFunction(NS::String::string("main0", NS::ASCIIStringEncoding));
  assert(vertex_shader);
  MTL::Function* fragment_shader = metal_backend_->metal_fragment_library->newFunction(NS::String::string("main0", NS::ASCIIStringEncoding));
  assert(fragment_shader);

  MTL::VertexDescriptor* vertex_descriptor = MTL::VertexDescriptor::vertexDescriptor();

  vertex_descriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat2);
  vertex_descriptor->attributes()->object(0)->setOffset(0);
  vertex_descriptor->attributes()->object(0)->setBufferIndex(1);

  vertex_descriptor->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
  vertex_descriptor->attributes()->object(1)->setOffset(8);
  vertex_descriptor->attributes()->object(1)->setBufferIndex(1);

  vertex_descriptor->attributes()->object(2)->setFormat(MTL::VertexFormatUInt);
  vertex_descriptor->attributes()->object(2)->setOffset(16);
  vertex_descriptor->attributes()->object(2)->setBufferIndex(1);

  vertex_descriptor->attributes()->object(3)->setFormat(MTL::VertexFormatUInt);
  vertex_descriptor->attributes()->object(3)->setOffset(20);
  vertex_descriptor->attributes()->object(3)->setBufferIndex(1);

  vertex_descriptor->layouts()->object(1)->setStride(sizeof(Vertex));
  vertex_descriptor->layouts()->object(1)->setStepFunction(MTL::VertexStepFunctionPerVertex);

  MTL::RenderPipelineDescriptor* render_pipeline_descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
  render_pipeline_descriptor->setLabel(NS::String::string("Bird Triangle Pipeline", NS::ASCIIStringEncoding));
  render_pipeline_descriptor->setVertexFunction(vertex_shader);
  render_pipeline_descriptor->setFragmentFunction(fragment_shader);
  render_pipeline_descriptor->setVertexDescriptor(vertex_descriptor);
  assert(render_pipeline_descriptor);
  MTL::PixelFormat pixel_format = (MTL::PixelFormat)metal_backend_->metal_layer->pixelFormat();
  render_pipeline_descriptor->colorAttachments()->object(0)->setPixelFormat(pixel_format);

  NS::Error* error;
  metal_backend_->metal_render_pso = metal_backend_->metal_device->newRenderPipelineState(render_pipeline_descriptor, &error);

  render_pipeline_descriptor->release();
  vertex_descriptor->release();
}

void MetalRenderer::draw() {
  send_render_command();
}

void MetalRenderer::send_render_command() {
  if (!metal_backend_ || !metal_backend_->metal_command_queue || !metal_backend_->metal_drawable) {
      return;
  }

  metal_backend_->metal_command_buffer = metal_backend_->metal_command_queue->commandBuffer();
  if (!metal_backend_->metal_command_buffer) return;

  MTL::RenderPassDescriptor* render_pass_descriptor = MTL::RenderPassDescriptor::alloc()->init();
  if (!render_pass_descriptor) return;

  MTL::RenderPassColorAttachmentDescriptor* color_descriptor = render_pass_descriptor->colorAttachments()->object(0);
  if (!color_descriptor) {
      render_pass_descriptor->release();
      return;
  }

  color_descriptor->setTexture(metal_backend_->metal_drawable->texture());
  color_descriptor->setLoadAction(MTL::LoadActionClear);
  color_descriptor->setClearColor(MTL::ClearColor(22.0f / 255.0f, 22.0f / 255.0f, 23.0f / 255.0f, 1.0));
  color_descriptor->setStoreAction(MTL::StoreActionStore);

  MTL::RenderCommandEncoder* render_command_encoder = metal_backend_->metal_command_buffer->renderCommandEncoder(render_pass_descriptor);
  if (!render_command_encoder) {
      render_pass_descriptor->release();
      return;
  }

  encode_render_command(render_command_encoder);
  render_command_encoder->endEncoding();

  metal_backend_->metal_command_buffer->presentDrawable(metal_backend_->metal_drawable);
  metal_backend_->metal_command_buffer->commit();
  metal_backend_->metal_command_buffer->waitUntilCompleted();
  render_pass_descriptor->release();
}

void MetalRenderer::encode_render_command(MTL::RenderCommandEncoder* render_command_encoder) {
  if (!render_command_encoder || !metal_backend_ || !metal_backend_->metal_render_pso || !metal_backend_->index_buffer || !metal_backend_->transform_buffer) {
      return;
  }
  render_command_encoder->setRenderPipelineState(metal_backend_->metal_render_pso);
    // MSL compiled from GLSL expects the uniform TransformBuffer at buffer(0).
    render_command_encoder->setVertexBuffer(metal_backend_->transform_buffer, 0, 0);
    // Vertex attributes are sourced from vertex buffer at buffer index 1.
    render_command_encoder->setVertexBuffer(metal_backend_->vertex_buffer, 0, 1);
  render_command_encoder->setFragmentTexture(texture->texture, 0);
  render_command_encoder->setFragmentSamplerState(metal_backend_->sampler_state, 0);
  render_command_encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, metal_backend_->index_buffer->length() / sizeof(uint16_t), MTL::IndexTypeUInt16, metal_backend_->index_buffer, 0);
}

void MetalRenderer::resizeFrameBuffer(int width, int height) {
  if (!metal_backend_ || !metal_backend_->metal_layer) {
      return;
  }

  if (width <= 0 || height <= 0) {
      return;
  }

  metal_backend_->metal_layer->setDrawableSize(CGSizeMake(width, height));
}

} // namespace bird

