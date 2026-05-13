#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

struct GLFWwindow;

#include "inputs/inputs.hpp"
#include "../utils/result.hpp"


namespace bird {

using namespace inputs;

struct WindowOptions {
  uint32_t width, height;
  std::string title;
};

class Window {
 public:
  using ResizeCallback = std::function<void(int width, int height)>;

  virtual ~Window() = default;

  virtual Result init() = 0;
  virtual void update() = 0;
  virtual Result close() = 0;

  virtual bool shouldClose() const = 0;

  virtual uint32_t getWidth() const = 0;
  virtual uint32_t getHeight() const = 0;
  virtual bool isFocused() const = 0;
  virtual bool isFullscreen() const = 0;
  virtual void setFullscreen(bool fullscreen) = 0;

  virtual Inputs& getInputs() = 0;
  virtual GLFWwindow* getNativeWindow() const = 0;
  virtual void setResizeCallback(ResizeCallback callback) = 0;

  /**
   * @brief Factory method to create the specific implementation
   */
  static std::unique_ptr<Window> create(const WindowOptions& options);
};

} // bird