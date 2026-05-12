#pragma once

#include "GLFW/glfw3.h"

#include "inputs.hpp"

namespace bird {
class GlfwWindow;
}

namespace bird::inputs {

/**
 * @brief Class for handling GLFW inputs
 */
class GlfwInputs : public Inputs {

  friend class ::bird::GlfwWindow;

 public:
  // Keyboard
  bool isKeyPressed(KeyCode key) override;    // True ONLY on the exact frame it was pressed
  bool isKeyHeld(KeyCode key) override;       // True as long as the key is down
  bool isKeyReleased(KeyCode key) override;   // True ONLY on the exact frame it was let go

  // Mouse
  bool isMouseButtonPressed(MouseCode button) override;
  bool isMouseButtonHeld(MouseCode button) override;
  bool isMouseButtonReleased(MouseCode button) override;
  glm::vec2 getMousePosition() override;
  float getMouseX() override;
  float getMouseY() override;
  glm::vec2 getMouseDelta() override;
  float getMouseDeltaX() override;
  float getMouseDeltaY() override;
  float getScrollX() override;
  float getScrollY() override;
  float getScrollDeltaX() override;
  float getScrollDeltaY() override;

  void endFrame() override;

 private:
  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
  static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
  static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
  void handleKey(int key, int scancode, int action, int mods);
  void handleMouseButton(int button, int action, int mods);
  void handleCursor(double xpos, double ypos);
  void handleScroll(double xoffset, double yoffset);

  // Mouse
  static const int MAX_MOUSE_BUTTONS = 16;
  bool currentMouseState[MAX_MOUSE_BUTTONS]{};
  bool previousMouseState[MAX_MOUSE_BUTTONS]{};

  float mouseX = 0.0f;
  float mouseY = 0.0f;
  float previousMouseX = 0.0f;
  float previousMouseY = 0.0f;

  float scrollX = 0.0f;
  float scrollY = 0.0f;
  float previousScrollX = 0.0f;
  float previousScrollY = 0.0f;

  // Keyboard
  static const int MAX_KEYS = 512;
  bool currentKeyState[MAX_KEYS]{};
  bool previousKeyState[MAX_KEYS]{};

  inline void endFrameKeys();
  inline void endFrameMouse();
};

} // bird::inputs