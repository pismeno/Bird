#if defined(BIRD_PLATFORM_WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(BIRD_PLATFORM_APPLE)
#define  GLFW_EXPOSE_NATIVE_COCOA
#endif

#include "glfw_window.hpp"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "osal/inputs/glfw/glfw_inputs.hpp"

namespace bird {

using namespace inputs;

GlfwWindow::GlfwWindow(const WindowOptions& options)
    : title(options.title),
      logical_width(options.logical_width),
      logical_height(options.logical_height),
      windowed_width(options.logical_width),
      windowed_height(options.logical_height),
      inputs(),
      glfw_window(nullptr),
      is_fullscreen_(options.start_fullscreen) {}

GlfwWindow::~GlfwWindow() = default;

Result<void> GlfwWindow::init() {
  glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  glfw_window = glfwCreateWindow(logical_width, logical_height, title.c_str(), nullptr, nullptr);

  if (!glfw_window) {
    return bird::fail("failed to create GLFW window");
  }

  if (this->is_fullscreen_) {
    set_fullscreen(true);
  }

  int fbWidth = 0, fbHeight = 0;
  glfwGetFramebufferSize(glfw_window, &fbWidth, &fbHeight);
  framebuffer_width = static_cast<uint32_t>(fbWidth);
  framebuffer_height = static_cast<uint32_t>(fbHeight);

  glfwSetWindowUserPointer(glfw_window, this);

  glfwSwapInterval(1); // Enable vsync

  glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);

  glfwSetWindowSizeCallback(glfw_window, [](GLFWwindow* window, int width, int height) {
    auto* bird_window = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    bird_window->window_resize_callback(bird_window->glfw_window, width, height);
  });

  glfwSetFramebufferSizeCallback(glfw_window, [](GLFWwindow* window, int width, int height) {
    auto* bird_window = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    bird_window->frame_buffer_resize_callback(bird_window->glfw_window, width, height);
  });

  glfwSetKeyCallback(glfw_window, GlfwInputs::key_callback);
  glfwSetMouseButtonCallback(glfw_window, GlfwInputs::mouse_button_callback);
  glfwSetCursorPosCallback(glfw_window, GlfwInputs::cursor_pos_callback);
  glfwSetScrollCallback(glfw_window, GlfwInputs::scrollCallback);

  return bird::ok();
}

void GlfwWindow::update() {
  glfwMakeContextCurrent(glfw_window);

  glfwPollEvents();
  glfwSwapBuffers(glfw_window);
}

Result<void> GlfwWindow::close() {
  glfwDestroyWindow(glfw_window);
  return bird::ok();
}

bool GlfwWindow::should_close() const noexcept {
  return glfwWindowShouldClose(glfw_window);
}

uint32_t GlfwWindow::get_logical_width() const {
  return logical_width;
}

uint32_t GlfwWindow::get_logical_height() const {
  return logical_height;
}

uint32_t GlfwWindow::get_framebuffer_width() const {
  return framebuffer_width;
}

uint32_t GlfwWindow::get_framebuffer_height() const {
  return framebuffer_height;
}

bool GlfwWindow::is_focused() const noexcept {
  return glfwGetWindowAttrib(glfw_window, GLFW_FOCUSED);
}

bool GlfwWindow::is_fullscreen() const noexcept {
  return is_fullscreen_;
}

Inputs& GlfwWindow::get_inputs() {
  return inputs;
}

void GlfwWindow::set_fullscreen(bool fullscreen) {
  if (fullscreen) {
    glfwGetWindowPos(glfw_window, &windowed_x, &windowed_y);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);

    is_fullscreen_ = true;
    glfwSetWindowMonitor(glfw_window, monitor, 0, 0, vidmode->width, vidmode->height, vidmode->refreshRate);
  } else {
    is_fullscreen_ = false;
    glfwSetWindowMonitor(glfw_window, nullptr, windowed_x, windowed_y,
                         windowed_width, windowed_height, 0);
  }
}

void GlfwWindow::window_resize_callback(GLFWwindow *window, int width, int height) {
  if (width <= 0 || height <= 0) return;

  logical_width = static_cast<uint32_t>(width);
  logical_height = static_cast<uint32_t>(height);

  if (!is_fullscreen()) {
    windowed_width = logical_width;
    windowed_height = logical_height;
  }
}

void GlfwWindow::frame_buffer_resize_callback(GLFWwindow *window, int width, int height) {
  if (width <= 0 || height <= 0) return;

  framebuffer_width = static_cast<uint32_t>(width);
  framebuffer_height = static_cast<uint32_t>(height);
}

void* GlfwWindow::get_native_handle() const {
#if defined(BIRD_PLATFORM_WINDOWS)
  return (void*) glfwGetWin32Window(glfw_window);
#elif defined(BIRD_PLATFORM_APPLE)
  return (void*) glfwGetCocoaWindow(glfwWindow);
#else
  return nullptr;
#endif
}

} // bird