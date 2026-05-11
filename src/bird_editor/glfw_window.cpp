#include "glfw_window.hpp"
#include "inputs/glfw_inputs.hpp"

namespace bird::editor {

GlfwWindow::GlfwWindow(int width, int height, const std::string& title) {
  this->data.width = width;
  this->data.height = height;
  this->data.title = title;

  this->glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}

GlfwWindow::~GlfwWindow() {}

void GlfwWindow::init() {
  glfwMakeContextCurrent(glfwWindow);

  glfwSwapInterval(1); // Enable vsync

  glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);

  glfwSetKeyCallback(glfwWindow, GlfwInputs::keyCallback);
  glfwSetMouseButtonCallback(glfwWindow, GlfwInputs::mouseButtonCallback);
  glfwSetCursorPosCallback(glfwWindow, GlfwInputs::cursorPosCallback);
  glfwSetScrollCallback(glfwWindow, GlfwInputs::scrollCallback);
}

void GlfwWindow::update() {
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

} // bird::editor