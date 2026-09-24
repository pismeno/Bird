#include <osal/iwindow.hpp>

#include <memory>

#include "osal/windowing/glfw/glfw_window.hpp"
#include <utils/result.hpp>

namespace bird {

Result<std::unique_ptr<IWindow>> IWindow::create(const WindowOptions& options) {
  auto window = std::unique_ptr<GlfwWindow>(new GlfwWindow(options));
  auto r_init = window->init();
  if (!r_init) return bird::fail(r_init.error());
  return std::move(window);
}

} // bird