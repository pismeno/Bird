#include "renderer.hpp"

#ifdef BIRD_PLATFORM_APPLE
#include "metal_renderer.hpp"
#endif

#include <memory>
#include <iostream>

namespace bird {

Renderer::~Renderer() {
  shutdown();
}

void Renderer::create_renderer(RendererType type) {
  renderer_.reset();

  switch (type) {
    case RendererType::Metal: {
#ifdef BIRD_PLATFORM_APPLE
      renderer_ = std::make_unique<MetalRenderer>();
#else
      std::cerr << "Metal Renderer is only supported on Apple platforms." << std::endl;
#endif
      break;
    }
    case RendererType::Vulkan: {
      // not yet implemented
      break;
    }
    case RendererType::DirectX: {
      // not yet implemented
      break;
    }
    default: {
      std::cerr << "Unsupported renderer type" << std::endl;
      break;
    }
  }
}

void Renderer::init(Window& window) {
  if (!renderer_) {
    std::cerr << "Renderer not created. Call create_renderer() first." << std::endl;
    return;
  }

  renderer_->init(window);
}

void Renderer::render() {
  if (!renderer_) {
    return;
  }

  renderer_->render();
}

void Renderer::submit_quad(const RenderCommand render_command) {
  if (!renderer_) {
    return;
  }

  renderer_->submit_quad(render_command);
}

void Renderer::shutdown() {
  if (!renderer_) {
    return;
  }

  renderer_->shutdown();
  renderer_.reset();
}

void Renderer::handle_window_resize(int width, int height) {
  if (!renderer_) {
    return;
  }

#ifdef BIRD_PLATFORM_APPLE
  auto* metal_renderer = dynamic_cast<MetalRenderer*>(renderer_.get());
  if (metal_renderer) {
    metal_renderer->resizeFrameBuffer(width, height);
  }
#else
  // On Windows/Linux, you'll eventually cast to Vulkan/DX12 here if they need explicit resizing
  // renderer_->handle_resize(width, height); // Consider making this a virtual function in base Renderer!
#endif
}

} // namespace bird