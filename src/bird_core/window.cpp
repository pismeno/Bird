#include "window.hpp"
#include "glfw_window.hpp"

namespace bird {

std::unique_ptr<Window> Window::create(const WindowOptions& options) {
  return std::make_unique<GlfwWindow>(options);
}

} // bird