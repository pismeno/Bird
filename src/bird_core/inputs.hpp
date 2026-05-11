#pragma once

#include <glm/vec2.hpp>

#include "input_codes.hpp"

namespace bird {

class Inputs {
 public:
  // Keyboard
  static bool isKeyPressed(KeyCode key);      // True ONLY on the exact frame it was pressed
  static bool isKeyHeld(KeyCode key);       // True as long as the key is down
  static bool isKeyReleased(KeyCode key);     // True ONLY on the exact frame it was let go

  // Mouse
  static bool isMouseButtonPressed(MouseCode button);
  static glm::vec2 getMousePosition();
  static float getMouseX();
  static float getMouseY();

 protected:
  static void update();
};

} // bird