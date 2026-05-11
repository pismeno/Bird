#pragma once

#include "glm/vec2.hpp"

#include "input_codes.hpp"

namespace bird {

class Inputs {
 public:
  // Keyboard
  static inline bool isKeyPressed(KeyCode key);      // True ONLY on the exact frame it was pressed
  static inline bool isKeyHeld(KeyCode key);       // True as long as the key is down
  static inline bool isKeyReleased(KeyCode key);     // True ONLY on the exact frame it was let go

  // Mouse
  static inline bool isMouseButtonPressed(MouseCode button);
  static inline bool isMouseButtonHeld(MouseCode button);
  static inline bool isMouseButtonReleased(MouseCode button);
  static inline glm::vec2 getMousePosition();
  static inline float getMouseX();
  static inline float getMouseY();
  static inline glm::vec2 getMouseDelta();
  static inline float getMouseDeltaX();
  static inline float getMouseDeltaY();
  static inline float getScrollX();
  static inline float getScrollY();
  static inline float getScrollDeltaX();
  static inline float getScrollDeltaY();

 protected:
  static void update();
};

} // bird