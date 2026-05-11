#include "window.hpp"
#include "glfw_window.hpp"

namespace bird {

std::unique_ptr<Window> Window::create(int width, int height, const std::string &title) {
  return std::make_unique<GlfwWindow>(width, height, title);
}

} // bird