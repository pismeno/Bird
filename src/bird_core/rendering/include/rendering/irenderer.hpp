#pragma once

#include <memory>

#include <cstdint>

#include <rendering/rendering_context.hpp>
#include <resources/render_command.hpp>
#include <utils/result.hpp>

namespace bird {

enum class RendererType {
  Metal,
  Vulkan,
  DirectX
};

class IRenderer {
public:
  explicit IRenderer() = default;
  virtual ~IRenderer() = default;

  virtual Result<void> attach_window(void* native_window_handle, uint32_t window_width, uint32_t window_height) = 0;
  virtual Result<void> init() = 0;
  virtual void render() = 0;
  virtual Result<void> shutdown() = 0;
  virtual void submit_quad(const RenderCommand render_command) = 0;
  virtual Result<void> resize_framebuffer(uint32_t width, uint32_t height) = 0;

  virtual bool needs_framebuffer_resize() const noexcept = 0;
  virtual bool is_context_lost() const noexcept = 0;

  /**
   * @brief Factory method to create the specific implementation
   */
  static Result<std::unique_ptr<IRenderer>> create(RendererType type, RenderingContext* rendering_context);
};

} // namespace bird