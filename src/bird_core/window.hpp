#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include "inputs/inputs.hpp"

namespace bird {

using namespace inputs;

class Window {
 public:
  virtual ~Window() = default;

  virtual void init() = 0;
  virtual void update() = 0;
  virtual void close() = 0;

  virtual bool shouldClose() const = 0;

  virtual uint32_t getWidth() const = 0;
  virtual uint32_t getHeight() const = 0;
  virtual bool isFocused() const = 0;
  virtual bool isFullscreen() const = 0;
  virtual void setFullscreen(bool fullscreen) = 0;

  virtual Inputs& getInputs() = 0;

  /**
   * @brief Factory method to create the specific implementation
   */
  static std::unique_ptr<Window> create(int width, int height, const std::string& title);
};

} // bird