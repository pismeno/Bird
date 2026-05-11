#pragma once

#include "glm/vec2.hpp"

#include "input_codes.hpp"

namespace bird {

/**
 * @brief Abstract static class for querying the engine about inputs
 */
class Inputs {
 public:
  // Keyboard
  static bool isKeyPressed(KeyCode key);      // True ONLY on the exact frame it was pressed
  static bool isKeyHeld(KeyCode key);       // True as long as the key is down
  static bool isKeyReleased(KeyCode key);     // True ONLY on the exact frame it was let go

  // Mouse
  static bool isMouseButtonPressed(MouseCode button);
  static bool isMouseButtonHeld(MouseCode button);
  static bool isMouseButtonReleased(MouseCode button);
  static glm::vec2 getMousePosition();
  static float getMouseX();
  static float getMouseY();
  static glm::vec2 getMouseDelta();
  static float getMouseDeltaX();
  static float getMouseDeltaY();
  static float getScrollX();
  static float getScrollY();
  static float getScrollDeltaX();
  static float getScrollDeltaY();

  static void endFrame();
};

} // bird