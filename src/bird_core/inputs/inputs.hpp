#pragma once

#include "glm/vec2.hpp"

#include "input_codes.hpp"

namespace bird::inputs {

/**
 * @brief Abstract static class for querying the engine about inputs
 */
class Inputs {
 public:
  explicit Inputs() = default;
  virtual ~Inputs() noexcept = default;

  // Keyboard
  virtual bool isKeyPressed(KeyCode key) noexcept = 0;    // True ONLY on the exact frame it was pressed
  virtual bool isKeyHeld(KeyCode key) noexcept = 0;       // True as long as the key is down
  virtual bool isKeyReleased(KeyCode key) noexcept = 0;   // True ONLY on the exact frame it was let go

  // Mouse
  virtual bool isMouseButtonPressed(MouseCode button) noexcept = 0;
  virtual bool isMouseButtonHeld(MouseCode button) noexcept = 0;
  virtual bool isMouseButtonReleased(MouseCode button) noexcept = 0;
  virtual glm::vec2 getMousePosition() noexcept = 0;
  virtual float getMouseX() noexcept = 0;
  virtual float getMouseY() noexcept = 0;
  virtual glm::vec2 getMouseDelta() noexcept = 0;
  virtual float getMouseDeltaX() noexcept = 0;
  virtual float getMouseDeltaY() noexcept = 0;
  virtual float getScrollX() noexcept = 0;
  virtual float getScrollY() noexcept = 0;
  virtual float getScrollDeltaX() noexcept = 0;
  virtual float getScrollDeltaY() noexcept = 0;

  virtual void endFrame() noexcept = 0;
};

} // bird::inputs