#include <bird_core/rendering/irenderer.hpp>

#ifdef BIRD_PLATFORM_APPLE
#include "rendering/metal/metal_renderer.hpp"
#endif

#include <bird_core/utils/result.hpp>
#include "rendering/vulkan/vulkan_renderer.hpp"

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
#ifdef BIRD_PLATFORM_WINDOWS
      return std::make_unique<VulkanRenderer>();
#else
      return bird::fail("Vulkan renderer is only supported on Windows");
#endif
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