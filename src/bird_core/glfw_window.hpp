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
  bool isFocused() const override;
  bool isFullscreen() const override;
  void setFullscreen(bool fullscreen) override;

 private:
  GLFWwindow* glfwWindow;

  struct WindowData {
    std::string title;
    uint32_t width, height;
    uint32_t windowedWidth, windowedHeight;
    int windowedX, windowedY;
    bool fullscreen;
  };

  WindowData data;

  void windowResizeCallback(GLFWwindow* window, int width, int height);
};

} // bird::editor