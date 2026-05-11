#include "glfw_window.hpp"
#include "inputs/glfw_inputs.hpp"

namespace bird {

using namespace inputs;

GlfwWindow::GlfwWindow(int width, int height, const std::string& title)
    : data{
          .title          = title,
          .width          = (uint32_t)width,
          .height         = (uint32_t)height,
          .windowedWidth  = (uint32_t)width,
          .windowedHeight = (uint32_t)height,
          .fullscreen     = false
      },
      inputs(),
      glfwWindow(nullptr)
{
  glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

  // TODO probably should check for errors here
}

GlfwWindow::~GlfwWindow() = default;

void GlfwWindow::init() {
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
}

void GlfwWindow::update() {
  glfwMakeContextCurrent(glfwWindow);

  glfwPollEvents();
  glfwSwapBuffers(glfwWindow);
}

void GlfwWindow::close() {
  glfwDestroyWindow(glfwWindow);
}

bool GlfwWindow::shouldClose() const {
  return glfwWindowShouldClose(glfwWindow);
}

uint32_t GlfwWindow::getWidth() const {
  return this->data.width;
}

uint32_t GlfwWindow::getHeight() const {
  return this->data.height;
}

bool GlfwWindow::isFocused() const {
  return glfwGetWindowAttrib(glfwWindow, GLFW_FOCUSED);
}

bool GlfwWindow::isFullscreen() const {
  return data.fullscreen;
}

Inputs& GlfwWindow::getInputs() {
  return inputs;
}

void GlfwWindow::setFullscreen(bool fullscreen) {
  if (fullscreen) {
    glfwGetWindowPos(glfwWindow, &data.windowedX, &data.windowedY);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);

    data.fullscreen = true;
    glfwSetWindowMonitor(glfwWindow, monitor, 0, 0, vidmode->width, vidmode->height, vidmode->refreshRate);
  } else {
    data.fullscreen = false;
    glfwSetWindowMonitor(glfwWindow, nullptr, data.windowedX, data.windowedY,
                         data.windowedWidth, data.windowedHeight, 0);
  }
}

void GlfwWindow::windowResizeCallback(GLFWwindow *window, int width, int height) {
  if (!isFullscreen()) {
    data.windowedWidth = width;
    data.windowedHeight = height;
  }

  data.width = width;
  data.height = height;
}

} // bird