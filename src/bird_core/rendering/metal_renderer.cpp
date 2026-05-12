#include "metal_renderer.hpp"

#include "GLFWBridge.hpp"

#include <CoreGraphics/CoreGraphics.h>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <simd/simd.h>

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace bird {

struct MetalRenderer::Impl {
    MTL::Device* metal_device = nullptr;
    GLFWwindow* glfw_window = nullptr;

    CA::MetalLayer* metal_layer = nullptr;
    CA::MetalDrawable* metal_drawable = nullptr;

    MTL::Library* metal_default_library = nullptr;
    MTL::CommandQueue* metal_command_queue = nullptr;
    MTL::CommandBuffer* metal_command_buffer = nullptr;
    MTL::RenderPipelineState* metal_render_pso = nullptr;
    MTL::Buffer* vertex_buffer = nullptr;
};

void MetalRenderer::init_device() {
    if (metal_backend_ == nullptr) {
        metal_backend_ = new Impl();
    }
    metal_backend_->metal_device = MTL::CreateSystemDefaultDevice();
}

void MetalRenderer::init_window() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    metal_backend_->glfw_window = glfwCreateWindow(1280, 720, "Bird Editor", nullptr, nullptr);

    if (!metal_backend_->glfw_window) {
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(metal_backend_->glfw_window, &width, &height);

    metal_backend_->metal_layer = CA::MetalLayer::layer();
    metal_backend_->metal_layer->setDevice(metal_backend_->metal_device);
    metal_backend_->metal_layer->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
    metal_backend_->metal_layer->setDrawableSize(CGSizeMake(width, height));

    GLFWBridge::AddLayerToWindow(metal_backend_->glfw_window, metal_backend_->metal_layer);
    glfwSetWindowUserPointer(metal_backend_->glfw_window, this);
    glfwSetFramebufferSizeCallback(metal_backend_->glfw_window, frame_buffer_size_callback);

}

void MetalRenderer::init() {
    init_device();
    init_window();

    create_triangle();
    create_default_library();
    create_command_queue();
    create_render_pipeline();
}

void MetalRenderer::run() {
    while (!glfwWindowShouldClose(metal_backend_->glfw_window)) {
        NS::AutoreleasePool* frame_pool = NS::AutoreleasePool::alloc()->init();
        metal_backend_->metal_drawable = metal_backend_->metal_layer->nextDrawable();
        if (metal_backend_->metal_drawable != nullptr) {
            draw();
        }
        frame_pool->release();
        glfwPollEvents();
    }
}

void MetalRenderer::shutdown() {
    if (metal_backend_ != nullptr && metal_backend_->glfw_window != nullptr) {
        glfwDestroyWindow(metal_backend_->glfw_window);
    }
    glfwTerminate();
    delete metal_backend_;
    metal_backend_ = nullptr;
}

void MetalRenderer::create_triangle() {
    simd::float3 triangle_vertices[] = {
        {-0.5f, -0.5f, 0.0f},
        { 0.5f, -0.5f, 0.0f},
        { 0.0f,  0.5f, 0.0f}
    };

    metal_backend_->vertex_buffer = metal_backend_->metal_device->newBuffer(&triangle_vertices, sizeof(triangle_vertices), MTL::ResourceStorageModeShared);
}

void MetalRenderer::create_default_library() {
    metal_backend_->metal_default_library = metal_backend_->metal_device->newDefaultLibrary();
    if (metal_backend_->metal_default_library) {
        return;
    }

    NS::Error* error = nullptr;

#ifdef BIRD_METAL_SHADER_SOURCE_PATH
    const char* shader_source_path = BIRD_METAL_SHADER_SOURCE_PATH;
#else
    const char* shader_source_path = "src/bird_core/rendering/triangle.metal";
#endif

    std::ifstream shader_file(shader_source_path);
    if (!shader_file.is_open()) {
        std::cerr << "Failed to open Metal shader source: " << shader_source_path;
        std::exit(-1);
    }

    std::ostringstream shader_source_stream;
    shader_source_stream << shader_file.rdbuf();
    const std::string shader_source = shader_source_stream.str();

    metal_backend_->metal_default_library = metal_backend_->metal_device->newLibrary(
        NS::String::string(shader_source.c_str(), NS::UTF8StringEncoding),
        nullptr,
        &error
    );

    if (!metal_backend_->metal_default_library) {
        std::cerr << "Failed to compile Metal library from source: " << shader_source_path;
        if (error != nullptr) {
            std::cerr << "\nMetal error: " << error->localizedDescription()->utf8String();
        }
        std::exit(-1);
    }
}

void MetalRenderer::create_command_queue() {
    metal_backend_->metal_command_queue = metal_backend_->metal_device->newCommandQueue();
}

void MetalRenderer::create_render_pipeline() {
    MTL::Function* vertex_shader = metal_backend_->metal_default_library->newFunction(NS::String::string("vertexShader", NS::ASCIIStringEncoding));
    assert(vertex_shader);
    MTL::Function* fragment_shader = metal_backend_->metal_default_library->newFunction(NS::String::string("fragmentShader", NS::ASCIIStringEncoding));
    assert(fragment_shader);

    MTL::RenderPipelineDescriptor* render_pipeline_descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    render_pipeline_descriptor->setLabel(NS::String::string("Bird Triangle Pipeline", NS::ASCIIStringEncoding));
    render_pipeline_descriptor->setVertexFunction(vertex_shader);
    render_pipeline_descriptor->setFragmentFunction(fragment_shader);
    assert(render_pipeline_descriptor);
    MTL::PixelFormat pixel_format = (MTL::PixelFormat)metal_backend_->metal_layer->pixelFormat();
    render_pipeline_descriptor->colorAttachments()->object(0)->setPixelFormat(pixel_format);

    NS::Error* error;
    metal_backend_->metal_render_pso = metal_backend_->metal_device->newRenderPipelineState(render_pipeline_descriptor, &error);

    render_pipeline_descriptor->release();
}

void MetalRenderer::draw() {
    send_render_command();
}

void MetalRenderer::send_render_command() {
    metal_backend_->metal_command_buffer = metal_backend_->metal_command_queue->commandBuffer();

    MTL::RenderPassDescriptor* render_pass_descriptor = MTL::RenderPassDescriptor::alloc()->init();
    MTL::RenderPassColorAttachmentDescriptor* color_descriptor = render_pass_descriptor->colorAttachments()->object(0);
    color_descriptor->setTexture(metal_backend_->metal_drawable->texture());
    color_descriptor->setLoadAction(MTL::LoadActionClear);
    color_descriptor->setClearColor(MTL::ClearColor(41.0f / 255.0f, 42.0f / 255.0f, 48.0f / 255.0f, 1.0));
    color_descriptor->setStoreAction(MTL::StoreActionStore);

    MTL::RenderCommandEncoder* render_command_encoder = metal_backend_->metal_command_buffer->renderCommandEncoder(render_pass_descriptor);
    encode_render_command(render_command_encoder);
    render_command_encoder->endEncoding();

    metal_backend_->metal_command_buffer->presentDrawable(metal_backend_->metal_drawable);
    metal_backend_->metal_command_buffer->commit();
    metal_backend_->metal_command_buffer->waitUntilCompleted();
    render_pass_descriptor->release();
}

void MetalRenderer::encode_render_command(MTL::RenderCommandEncoder* render_command_encoder) {
    render_command_encoder->setRenderPipelineState(metal_backend_->metal_render_pso);
    render_command_encoder->setVertexBuffer(metal_backend_->vertex_buffer, 0, 0);
    MTL::PrimitiveType type_triangle = MTL::PrimitiveTypeTriangle;
    NS::UInteger vertex_start = 0;
    NS::UInteger vertex_count = 3;
    render_command_encoder->drawPrimitives(type_triangle, vertex_start, vertex_count);
}

void MetalRenderer::frame_buffer_size_callback(GLFWwindow *window, int width, int height) {
    MetalRenderer* renderer = (MetalRenderer*)(glfwGetWindowUserPointer(window));
    if (renderer) {
        renderer->resize_frame_buffer(width, height);
    }
}

void MetalRenderer::resize_frame_buffer(int width, int height) {
    metal_backend_->metal_layer->setDrawableSize(CGSizeMake(width, height));
}

} // namespace bird
