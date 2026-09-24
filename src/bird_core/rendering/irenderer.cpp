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

namespace {
Result<std::unique_ptr<IRenderer>> make_renderer(RendererType type) {
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
}

Result<std::unique_ptr<IRenderer>> IRenderer::create(RendererOptions options, RenderingContext& context) {
  auto r_make_renderer = make_renderer(options.type);
  if (!r_make_renderer) return r_make_renderer;
  std::unique_ptr<IRenderer> renderer = std::move(r_make_renderer).value();

  auto r_attach_window = renderer->attach_window(options.native_window_handle, options.window_width, options.window_height);
  if (!r_attach_window) return bird::fail(r_attach_window.error());

  auto r_init = renderer->init(context);
  if (!r_init) return bird::fail(r_init.error());

  return std::move(renderer);
}

} // bird