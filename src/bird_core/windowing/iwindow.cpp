#include "iwindow.hpp"
#include "glfw_window.hpp"

namespace bird {

std::unique_ptr<IWindow> IWindow::create(const WindowOptions& options) {
  return std::make_unique<GlfwWindow>(options);
}

} // bird