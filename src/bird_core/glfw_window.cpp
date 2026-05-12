#include "glfw_window.hpp"

#include <utility>
#include "inputs/glfw_inputs.hpp"

namespace bird {

using namespace inputs;

GlfwWindow::GlfwWindow(WindowOptions options)
    : options(std::move(options)),
      inputs(),
      windowedWidth(options.width),
      windowedHeight(options.height),
      glfwWindow(nullptr),
      fullscreen(false) {}

GlfwWindow::~GlfwWindow() = default;

Result GlfwWindow::init() {
  glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  glfwWindow = glfwCreateWindow(options.width, options.height, options.title.c_str(), nullptr, nullptr);

  if (!glfwWindow) {
    return Result::fail("failed to create GLFW window");
  }

  glfwSetWindowUserPointer(glfwWindow, this);

  glfwSetWindowSizeCallback(glfwWindow, [](GLFWwindow* window, int width, int height) {
    auto* birdWindow = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    birdWindow->windowResizeCallback(birdWindow->glfwWindow, width, height);
  });

  glfwSetFramebufferSizeCallback(glfwWindow, [](GLFWwindow* window, int width, int height) {
    auto* birdWindow = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    birdWindow->frameBufferResizeCallback(birdWindow->glfwWindow, width, height);
  });

  glfwSetKeyCallback(glfwWindow, GlfwInputs::keyCallback);
  glfwSetMouseButtonCallback(glfwWindow, GlfwInputs::mouseButtonCallback);
  glfwSetCursorPosCallback(glfwWindow, GlfwInputs::cursorPosCallback);
  glfwSetScrollCallback(glfwWindow, GlfwInputs::scrollCallback);

  return Result::ok();
}

void GlfwWindow::update() {
  glfwPollEvents();
}

Result GlfwWindow::close() {
  glfwDestroyWindow(glfwWindow);
  return Result::ok();
}

bool GlfwWindow::shouldClose() const {
  return glfwWindowShouldClose(glfwWindow);
}

uint32_t GlfwWindow::getWidth() const {
  return this->options.width;
}

uint32_t GlfwWindow::getHeight() const {
  return this->options.height;
}

bool GlfwWindow::isFocused() const {
  return glfwGetWindowAttrib(glfwWindow, GLFW_FOCUSED);
}

bool GlfwWindow::isFullscreen() const {
  return fullscreen;
}

Inputs& GlfwWindow::getInputs() {
  return inputs;
}

GLFWwindow* GlfwWindow::getNativeWindow() const {
  return glfwWindow;
}

void GlfwWindow::setResizeCallback(ResizeCallback callback) {
  resize_callback = std::move(callback);

  if (glfwWindow && resize_callback) {
    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(glfwWindow, &framebufferWidth, &framebufferHeight);
    if (framebufferWidth > 0 && framebufferHeight > 0) {
      options.width = static_cast<uint32_t>(framebufferWidth);
      options.height = static_cast<uint32_t>(framebufferHeight);
      resize_callback(framebufferWidth, framebufferHeight);
    }
  }
}

void GlfwWindow::setFullscreen(bool fullscreen) {
  if (fullscreen) {
    glfwGetWindowPos(glfwWindow, &windowedX, &windowedY);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);

    fullscreen = true;
    glfwSetWindowMonitor(glfwWindow, monitor, 0, 0, vidmode->width, vidmode->height, vidmode->refreshRate);
  } else {
    fullscreen = false;
    glfwSetWindowMonitor(glfwWindow, nullptr, windowedX, windowedY,
                         windowedWidth, windowedHeight, 0);
  }
}

void GlfwWindow::windowResizeCallback(GLFWwindow *window, int width, int height) {
  if (!isFullscreen()) {
    windowedWidth = width;
    windowedHeight = height;
  }
}

void GlfwWindow::frameBufferResizeCallback(GLFWwindow *window, int width, int height) {
  if (width <= 0 || height <= 0) {
    return;
  }

  options.width = static_cast<uint32_t>(width);
  options.height = static_cast<uint32_t>(height);

  if (resize_callback) {
    resize_callback(width, height);
  }
}

} // bird