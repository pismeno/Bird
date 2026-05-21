#include "inputs/glfw_inputs.hpp"
#include "inputs.hpp"

#include "GLFW/glfw3.h"
#include "../windowing/glfw_window.hpp"

namespace bird::inputs {

bool GlfwInputs::isKeyHeld(KeyCode key) noexcept {
  return currentKeyState[(uint16_t) key];
}

bool GlfwInputs::isKeyPressed(KeyCode key) noexcept {
  return currentKeyState[(uint16_t) key] && !previousKeyState[(uint16_t) key];
}

bool GlfwInputs::isKeyReleased(KeyCode key) noexcept {
  return !currentKeyState[(uint16_t) key] && previousKeyState[(uint16_t) key];
}

bool GlfwInputs::isMouseButtonHeld(MouseCode button) noexcept {
  return currentMouseState[(uint16_t)button];
}

bool GlfwInputs::isMouseButtonPressed(MouseCode button) noexcept {
  return currentMouseState[(uint16_t)button] && !previousMouseState[(uint16_t)button];
}

bool GlfwInputs::isMouseButtonReleased(MouseCode button) noexcept {
  return !currentMouseState[(uint16_t)button] && previousMouseState[(uint16_t)button];
}

glm::vec2 GlfwInputs::getMousePosition() noexcept {
  return glm::vec2(mouseX, mouseY);
}

float GlfwInputs::getMouseX() noexcept {
  return mouseX;
}

float GlfwInputs::getMouseY() noexcept {
  return mouseY;
}

glm::vec2 GlfwInputs::getMouseDelta() noexcept {
  return glm::vec2(getMouseDeltaX(), getMouseDeltaY());
}

float GlfwInputs::getMouseDeltaY() noexcept {
  return mouseY - previousMouseY;
}

float GlfwInputs::getMouseDeltaX() noexcept {
  return mouseX - previousMouseX;
}

float GlfwInputs::getScrollX() noexcept {
  return scrollX;
}

float GlfwInputs::getScrollY() noexcept {
  return scrollY;
}

float GlfwInputs::getScrollDeltaX() noexcept {
  return scrollX - previousScrollX;
}

float GlfwInputs::getScrollDeltaY() noexcept {
  return scrollY - previousScrollY;
}

void GlfwInputs::endFrame() noexcept {
  endFrameKeys();
  endFrameMouse();
}

inline void GlfwInputs::endFrameKeys() noexcept {
  for (int i = 0; i < MAX_KEYS; i++) {
    previousKeyState[i] = currentKeyState[i];
  }
}

inline void GlfwInputs::endFrameMouse() noexcept {
  for (int i = 0; i < MAX_MOUSE_BUTTONS; i++) {
    previousMouseState[i] = currentMouseState[i];
  }

  previousMouseX = mouseX;
  previousMouseY = mouseY;

  scrollX = 0.0f;
  scrollY = 0.0f;
  previousScrollX = 0.0f;
  previousScrollY = 0.0f;
}

void GlfwInputs::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  auto* birdWindow = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));

  if (birdWindow) {
    birdWindow->inputs.handleKey(key, scancode, action, mods);
  }
}

void GlfwInputs::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
  auto* birdWindow = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));

  if (birdWindow) {
    birdWindow->inputs.handleMouseButton(button, action, mods);
  }
}

void GlfwInputs::cursorPosCallback(GLFWwindow *window, double xpos, double ypos) {
  auto* birdWindow = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));

  if (birdWindow) {
    birdWindow->inputs.handleCursor(xpos, ypos);
  }
}

void GlfwInputs::scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
  auto* birdWindow = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));

  if (birdWindow) {
    birdWindow->inputs.handleScroll(xoffset, yoffset);
  }
}

void GlfwInputs::handleKey(int key, int scancode, int action, int mods) noexcept {
  if (key < 0 || key >= MAX_KEYS) {
    return; // Ignore out-of-bounds keys
  }

  // Converting to local key codes is not needed, since GLFW codes are matching with ours
  if (action == GLFW_PRESS) {
    currentKeyState[key] = true;
  } else if (action == GLFW_RELEASE) {
    currentKeyState[key] = false;
  }
}

void GlfwInputs::handleMouseButton(int button, int action, int mods) noexcept {
  if (button < 0 || action >= MAX_MOUSE_BUTTONS) {
    return; // Ignore out-of-bounds keys
  }

  // Converting to local key codes is not needed, since GLFW codes are matching with ours
  if (action == GLFW_PRESS) {
    currentMouseState[button] = true;
  } else if (action == GLFW_RELEASE) {
    currentMouseState[button] = false;
  }
}

void GlfwInputs::handleCursor(double xpos, double ypos) noexcept {
  auto x = (float)xpos;
  auto y = (float)ypos;

  mouseX = x;
  mouseY = y;
}

void GlfwInputs::handleScroll(double xoffset, double yoffset) noexcept {
  auto x = (float)xoffset;
  auto y = (float)yoffset;

  scrollX = x;
  scrollY = y;
}

} // bird::inputs