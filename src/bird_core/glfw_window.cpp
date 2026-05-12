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
  glfwWindow = glfwCreateWindow(options.width, options.height, options.title.c_str(), nullptr, nullptr);

  if (!glfwWindow) {
    return Result::fail("failed to create GLFW window");
  }

  glfwMakeContextCurrent(glfwWindow);

  glfwSetWindowUserPointer(glfwWindow, this);

  glfwSwapInterval(1); // Enable vsync

  glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);

  glfwSetWindowSizeCallback(glfwWindow, [](GLFWwindow* window, int width, int height) {
    auto* birdWindow = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    birdWindow->windowResizeCallback(birdWindow->glfwWindow, width, height);
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

  options.width = width;
  options.height = height;
}

} // bird