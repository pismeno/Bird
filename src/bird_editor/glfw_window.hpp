#pragma once

#include "window.hpp"

#include "GLFW/glfw3.h"

namespace bird::editor {

class GlfwWindow : public Window {
 public:
  GlfwWindow(int width, int height, const std::string& title);
  virtual ~GlfwWindow();

  void init() override;
  void update() override;
  void close() override;
  bool shouldClose() const override;
  uint32_t getWidth() const override;
  uint32_t getHeight() const override;

 private:
  GLFWwindow* glfwWindow;

  struct WindowData {
    std::string title;
    uint32_t width, height;
  };

  WindowData data;
};

} // bird::editor