#pragma once

#include "window.hpp"

#include "GLFW/glfw3.h"

#include "../inputs/glfw_inputs.hpp"
#include "../utils/result.hpp"

namespace bird {

using namespace inputs;

class GlfwWindow : public Window {

  friend class GlfwInputs;

 public:
  GlfwWindow(WindowOptions options);
  virtual ~GlfwWindow();

  Result init() override;
  void update() override;
  Result close() override;
  bool shouldClose() const override;
  uint32_t getWidth() const override;
  uint32_t getHeight() const override;
  bool isFocused() const override;
  bool isFullscreen() const override;
  void setFullscreen(bool fullscreen) override;
  Inputs& getInputs() override;
  GLFWwindow* getNativeWindow() const override;
  void setResizeCallback(ResizeCallback callback) override;

 private:
  GLFWwindow* glfwWindow;

  uint32_t windowedWidth{}, windowedHeight{};
  int windowedX{}, windowedY{};
  bool fullscreen{};

  WindowOptions options;

  GlfwInputs inputs;
  ResizeCallback resize_callback;

  void windowResizeCallback(GLFWwindow* window, int width, int height);
  void frameBufferResizeCallback(GLFWwindow* window, int width, int height);
};

} // bird