#include "rendering/irenderer.hpp"

#ifdef BIRD_PLATFORM_APPLE
#include "rendering/metal_renderer.hpp"
#endif

#include "utils/result.hpp"

namespace bird {

Result<std::unique_ptr<IRenderer>> IRenderer::create(RendererType type) {
  switch (type) {
    case RendererType::Metal: {
#ifdef BIRD_PLATFORM_APPLE
      return std::make_unique<MetalRenderer>();
#else
      return bird::fail("Metal renderer is only supported on macOS");
#endif
    }
    case RendererType::Vulkan: {
      return bird::fail("Vulkan renderer is not yet implemented");
    }
    case RendererType::DirectX: {
      return bird::fail("DirectX renderer is not yet implemented");
    }
    default: {
      return bird::fail("Unsupported renderer type");
    }
  }
}

} // bird