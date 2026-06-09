#pragma once

#include <memory>

#include "rendering/render_command.hpp"
#include "utils/result.hpp"

namespace bird {

class IWindow;

enum class RendererType {
  Metal,
  Vulkan,
  DirectX
};

class IRenderer {
public:
  explicit IRenderer() = default;
  virtual ~IRenderer() = default;

  virtual Result<void> init(IWindow& window) = 0;
  virtual void render() = 0;
  virtual Result<void> shutdown() = 0;
  virtual void submit_quad(const RenderCommand render_command) = 0;
  virtual void handle_window_resize(int width, int height) = 0; // TODO actually fix the implementation with the new IWindow

  /**
   * @brief Factory method to create the specific implementation
   */
  static Result<std::unique_ptr<IRenderer>> create(RendererType type);
};

} // namespace bird