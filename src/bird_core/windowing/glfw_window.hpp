#pragma once

#include "window.hpp"

#include "GLFW/glfw3.h"

#include "inputs/glfw_inputs.hpp"
#include "utils/result.hpp"

namespace bird {

using namespace inputs;

class GlfwWindow : public Window {

  friend class inputs::GlfwInputs;

 public:
  GlfwWindow(const WindowOptions& options);
  virtual ~GlfwWindow();

  Result init() override;
  void update() override;
  Result close() override;
  bool shouldClose() const override;
  uint32_t getLogicalWidth() const override;
  uint32_t getLogicalHeight() const override;
  uint32_t getFramebufferWidth() const override;
  uint32_t getFramebufferHeight() const override;
  bool isFocused() const override;
  bool isFullscreen() const override;
  void setFullscreen(bool fullscreen) override;
  Inputs& getInputs() override;
  void* getNativeHandle() const override;

 private:
  GLFWwindow* glfwWindow{};

  std::string title{};

  uint32_t logicalWidth{};
  uint32_t logicalHeight{};
  uint32_t framebufferWidth{};
  uint32_t framebufferHeight{};

  uint32_t windowedWidth{};
  uint32_t windowedHeight{};
  int windowedX{}, windowedY{};
  bool fullscreen{};

  GlfwInputs inputs;

  void windowResizeCallback(GLFWwindow* window, int width, int height);
  void frameBufferResizeCallback(GLFWwindow* window, int width, int height);
};

} // bird