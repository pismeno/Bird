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
  virtual ~GlfwWindow() noexcept;

  Result<void> init() override;
  void update() override;
  Result<void> close() override;
  bool shouldClose() const noexcept override;
  uint32_t getWidth() const noexcept override;
  uint32_t getHeight() const noexcept override;
  bool isFocused() const noexcept override;
  bool isFullscreen() const noexcept override;
  void setFullscreen(bool fullscreen) noexcept override;
  Inputs& getInputs() noexcept override;

 private:
  GLFWwindow* glfwWindow;

  uint32_t windowedWidth{}, windowedHeight{};
  int windowedX{}, windowedY{};
  bool fullscreen{};

  WindowOptions options;

  GlfwInputs inputs;

  void windowResizeCallback(GLFWwindow* window, int width, int height) noexcept;
};

} // bird