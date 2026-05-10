#include "glfw_window.hpp"

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