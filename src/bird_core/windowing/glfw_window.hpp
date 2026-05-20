#pragma once

#include "iwindow.hpp"

#include "GLFW/glfw3.h"

#include "inputs/glfw_inputs.hpp"
#include "utils/result.hpp"

namespace bird {

using namespace inputs;

class GlfwWindow : public IWindow {

  friend class GlfwInputs;

 public:
  explicit GlfwWindow(WindowOptions options);
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

 private:
  GLFWwindow* glfwWindow;

  uint32_t windowedWidth{}, windowedHeight{};
  int windowedX{}, windowedY{};
  bool fullscreen{};

  WindowOptions options;

  GlfwInputs inputs;

  void windowResizeCallback(GLFWwindow* window, int width, int height);
};

} // bird