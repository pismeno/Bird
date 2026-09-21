#include <os/iwindow.hpp>

#include <memory>

#include "os/windowing/glfw/glfw_window.hpp"
#include <utils/result.hpp>

namespace bird {

Result<std::unique_ptr<IWindow>> IWindow::create(const WindowOptions& options) {
  return std::make_unique<GlfwWindow>(options);
}

} // bird