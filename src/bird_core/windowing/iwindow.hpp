#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "inputs/inputs.hpp"
#include "utils/result.hpp"

namespace bird {

using namespace inputs;

struct WindowOptions {
  uint32_t logical_width, logical_height;

  std::string title;

  bool startFullscreen = false;
};

class IWindow {
 public:
  explicit IWindow() = default;
  virtual ~IWindow() noexcept = default;

  virtual Result<void> init() = 0;
  virtual void update() = 0;
  virtual Result<void> close() = 0;

  virtual bool shouldClose() const noexcept = 0;

  virtual uint32_t getLogicalWidth() const = 0;
  virtual uint32_t getLogicalHeight() const = 0;
  virtual uint32_t getFramebufferWidth() const = 0;
  virtual uint32_t getFramebufferHeight() const = 0;
  virtual bool isFocused() const noexcept = 0;
  virtual bool isFullscreen() const noexcept = 0;
  virtual void setFullscreen(bool fullscreen) = 0;

  virtual Inputs& getInputs() = 0;

  virtual void* getNativeHandle() const = 0;

  /**
   * @brief Factory method to create the specific implementation
   */
  static Result<std::unique_ptr<IWindow>> create(const WindowOptions& options);
};

} // bird