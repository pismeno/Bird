#include <rendering/irenderer.hpp>

#ifdef BIRD_PLATFORM_APPLE
#include "rendering/metal/metal_renderer.hpp"
#endif

#ifdef BIRD_PLATFORM_WINDOWS
#include "rendering/vulkan/vulkan_renderer.hpp"
#endif

#include <utils/result.hpp>
#include <rendering/rendering_context.hpp>

namespace bird {

Result<std::unique_ptr<IRenderer>> IRenderer::create(RendererOptions options, RenderingContext& context) {
  switch (options.type) {
    case RendererType::Metal: {
#ifdef BIRD_PLATFORM_APPLE
      return std::make_unique<MetalRenderer>();
#else
      return bird::fail("Metal renderer is only supported on macOS");
#endif
    }
    case RendererType::Vulkan: {
#ifdef BIRD_PLATFORM_WINDOWS
      auto r_vulkan = VulkanRenderer::create(context, options.native_window_handle, options.window_width, options.window_height);
      if (!r_vulkan) return bird::fail(r_vulkan.error());

      return std::unique_ptr<IRenderer>(std::move(r_vulkan).value());
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