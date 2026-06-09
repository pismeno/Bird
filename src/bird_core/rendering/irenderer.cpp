#include "rendering/irenderer.hpp"

#include <iostream>

#ifdef BIRD_PLATFORM_APPLE
#include "rendering/metal_renderer.hpp"
#endif

namespace bird {

std::unique_ptr<IRenderer> IRenderer::create(RendererType type) {
  switch (type) {
    case RendererType::Metal: {
#ifdef BIRD_PLATFORM_APPLE
      return std::make_unique<MetalRenderer>();
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
      return nullptr;
    }
  }

  return nullptr;
}

} // bird