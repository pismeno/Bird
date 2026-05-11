#include "inputs/glfw_inputs.hpp"
#include "inputs.hpp"

#include "GLFW/glfw3.h"

namespace bird {

static const int MAX_MOUSE_BUTTONS = 16;
static bool currentMouseState[MAX_MOUSE_BUTTONS];
static bool previousMouseState[MAX_MOUSE_BUTTONS];

static float mouseX = 0.0f;
static float mouseY = 0.0f;
static float previousMouseX = 0.0f;
static float previousMouseY = 0.0f;

static float scrollX = 0.0f;
static float scrollY = 0.0f;
static float previousScrollX = 0.0f;
static float previousScrollY = 0.0f;

bool Inputs::isMouseButtonHeld(MouseCode button) {
  return currentMouseState[(uint16_t)button];
}

bool Inputs::isMouseButtonPressed(MouseCode button) {
  return currentMouseState[(uint16_t)button] && !previousMouseState[(uint16_t)button];
}

bool Inputs::isMouseButtonReleased(MouseCode button) {
  return !currentMouseState[(uint16_t)button] && previousMouseState[(uint16_t)button];
}

glm::vec2 Inputs::getMousePosition() {
  return glm::vec2(mouseX, mouseY);
}

float Inputs::getMouseX() {
  return mouseX;
}

float Inputs::getMouseY() {
  return mouseY;
}

glm::vec2 Inputs::getMouseDelta() {
  return glm::vec2(getMouseDeltaX(), getMouseDeltaY());
}

float Inputs::getMouseDeltaY() {
  return mouseY - previousMouseY;
}

float Inputs::getMouseDeltaX() {
  return mouseX - previousMouseX;
}

float Inputs::getScrollX() {
  return scrollX;
}

float Inputs::getScrollY() {
  return scrollY;
}

float Inputs::getScrollDeltaX() {
  return scrollX - previousScrollX;
}

float Inputs::getScrollDeltaY() {
  return scrollY - previousScrollY;
}

void GlfwInputs::endFrameMouse() {
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

void GlfwInputs::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
  if (button < 0 || action >= MAX_MOUSE_BUTTONS) {
    return;
  }

  if (action == GLFW_PRESS) {
    currentMouseState[button] = true;
  } else if (action == GLFW_RELEASE) {
    currentMouseState[button] = false;
  }
}

void GlfwInputs::cursorPosCallback(GLFWwindow *window, double xpos, double ypos) {
  float x = (float)xpos;
  float y = (float)ypos;

  mouseX = x;
  mouseY = y;
}

void GlfwInputs::scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
  float x = (float)xoffset;
  float y = (float)yoffset;

  scrollX = x;
  scrollY = y;
}

} // bird