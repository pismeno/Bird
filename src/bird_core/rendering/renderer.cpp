#include "renderer.hpp"
#include "metal_renderer.hpp"

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
        renderer_ = std::make_unique<MetalRenderer>();
        break;
    }
    case RendererType::Vulkan: {
        // not yet implemented
        // renderer = bird::VulkanRenderer();
        break;
    }
    case RendererType::DirectX: {
        // not yet implemented
        // renderer = bird::DirectXRenderer();
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

  auto* metal_renderer = dynamic_cast<MetalRenderer*>(renderer_.get());
  if (metal_renderer) {
    metal_renderer->resizeFrameBuffer(width, height);
  }
}

} // namespace bird
