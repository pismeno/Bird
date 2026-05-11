#pragma once

#include "glm/vec2.hpp"

#include "input_codes.hpp"

namespace bird::inputs {

/**
 * @brief Abstract static class for querying the engine about inputs
 */
class Inputs {
 public:
  // Keyboard
  virtual bool isKeyPressed(KeyCode key) = 0;    // True ONLY on the exact frame it was pressed
  virtual bool isKeyHeld(KeyCode key) = 0;       // True as long as the key is down
  virtual bool isKeyReleased(KeyCode key) = 0;   // True ONLY on the exact frame it was let go

  // Mouse
  virtual bool isMouseButtonPressed(MouseCode button) = 0;
  virtual bool isMouseButtonHeld(MouseCode button) = 0;
  virtual bool isMouseButtonReleased(MouseCode button) = 0;
  virtual glm::vec2 getMousePosition() = 0;
  virtual float getMouseX() = 0;
  virtual float getMouseY() = 0;
  virtual glm::vec2 getMouseDelta() = 0;
  virtual float getMouseDeltaX() = 0;
  virtual float getMouseDeltaY() = 0;
  virtual float getScrollX() = 0;
  virtual float getScrollY() = 0;
  virtual float getScrollDeltaX() = 0;
  virtual float getScrollDeltaY() = 0;

  virtual void endFrame() = 0;
};

} // bird::inputs