#pragma once

#include <memory>

#include "irenderer.hpp"
#include "windowing/iwindow.hpp"

namespace bird {

enum class RendererType {
  Metal,
  Vulkan,
  DirectX
};

class Renderer {
public:
  ~Renderer();

  /**
  * @brief Factory method to create the specific implementation
  */

  void create_renderer(RendererType type);
  void init(Window& window);
  void render();
  void shutdown();
  void submit_quad(const RenderCommand render_command);
  void handle_window_resize(int width, int height);

private:
  std::unique_ptr<IRenderer> renderer_;

};

}