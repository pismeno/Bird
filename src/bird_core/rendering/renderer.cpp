#include "renderer.hpp"
#include "metal_renderer.hpp"

#include <memory>
#include <iostream>

namespace bird {

Renderer::~Renderer() {
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

void Renderer::init() {
    if (!renderer_) {
        std::cerr << "Renderer not created. Call create_renderer() first." << std::endl;
        return;
    }

    renderer_->init();
    renderer_->run();
    renderer_->shutdown();
}


} // namespace bird
