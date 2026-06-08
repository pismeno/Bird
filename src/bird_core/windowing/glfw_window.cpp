#include "glfw_window.hpp"

#include <utility>

#if defined(BIRD_PLATFORM_WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(BIRD_PLATFORM_APPLE)
#define  GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "inputs/glfw_inputs.hpp"

namespace bird {

using namespace inputs;

GlfwWindow::GlfwWindow(const WindowOptions& options)
    : title(options.title),
      logicalWidth(options.logical_width),
      logicalHeight(options.logical_height),
      windowedWidth(options.logical_width),
      windowedHeight(options.logical_height),
      inputs(),
      windowedWidth(options.width),
      windowedHeight(options.height),
      glfwWindow(nullptr),
      fullscreen(options.startFullscreen) {}

GlfwWindow::~GlfwWindow() = default;

Result GlfwWindow::init() {
  glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  glfwWindow = glfwCreateWindow(logicalWidth, logicalHeight, title.c_str(), nullptr, nullptr);

  if (!glfwWindow) {
    return Result::fail("failed to create GLFW window");
  }

  if (this->fullscreen) {
    setFullscreen(true);
  }

  int fbWidth = 0, fbHeight = 0;
  glfwGetFramebufferSize(glfwWindow, &fbWidth, &fbHeight);
  framebufferWidth = static_cast<uint32_t>(fbWidth);
  framebufferHeight = static_cast<uint32_t>(fbHeight);

  glfwSetWindowUserPointer(glfwWindow, this);

  glfwSwapInterval(1); // Enable vsync

  glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);

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
  glfwMakeContextCurrent(glfwWindow);

  glfwPollEvents();
  glfwSwapBuffers(glfwWindow);
}

Result GlfwWindow::close() {
  glfwDestroyWindow(glfwWindow);
  return Result::ok();
}

bool GlfwWindow::shouldClose() const {
  return glfwWindowShouldClose(glfwWindow);
}

uint32_t GlfwWindow::getLogicalWidth() const {
  return logicalWidth;
}

uint32_t GlfwWindow::getLogicalHeight() const {
  return logicalHeight;
}

uint32_t GlfwWindow::getFramebufferWidth() const {
  return framebufferWidth;
}

uint32_t GlfwWindow::getFramebufferHeight() const {
  return framebufferHeight;
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

void GlfwWindow::setFullscreen(bool goFullscreen) {
  if (goFullscreen) {
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
  if (width <= 0 || height <= 0) return;

  logicalWidth = static_cast<uint32_t>(width);
  logicalHeight = static_cast<uint32_t>(height);

  if (!isFullscreen()) {
    windowedWidth = logicalWidth;
    windowedHeight = logicalHeight;
  }
}

void GlfwWindow::frameBufferResizeCallback(GLFWwindow *window, int width, int height) {
  if (width <= 0 || height <= 0) return;

  framebufferWidth = static_cast<uint32_t>(width);
  framebufferHeight = static_cast<uint32_t>(height);
}

void* GlfwWindow::getNativeHandle() const {
#if defined(BIRD_PLATFORM_WINDOWS)
  return (void*) glfwGetWin32Window(glfwWindow);
#elif defined(BIRD_PLATFORM_APPLE)
  return (void*) glfwGetCocoaWindow(glfwWindow);
#else
  return nullptr;
#endif
}

} // bird