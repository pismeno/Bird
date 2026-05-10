#include "glfw_window.hpp"

namespace bird::editor {

GlfwWindow::GlfwWindow(int width, int height, const std::string& title) {
  this->data.width = width;
  this->data.height = height;
  this->data.title = title;

  this->window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

  glfwMakeContextCurrent(window);
}

GlfwWindow::~GlfwWindow() {}

void GlfwWindow::onUpdate() {
  glfwPollEvents();
  glfwSwapBuffers(window);
}

bool GlfwWindow::shouldClose() const {
  return glfwWindowShouldClose(window);
}

uint32_t GlfwWindow::getWidth() const {
  return this->data.width;
}

uint32_t GlfwWindow::getHeight() const {
  return this->data.height;
}

} // bird::editor