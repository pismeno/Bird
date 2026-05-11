#include "glfw_inputs.hpp"
#include "inputs.hpp"

#include <GLFW/glfw3.h>

namespace bird {

static const int MAX_KEYS = 512;
static bool currentKeyState[MAX_KEYS];
static bool previousKeyState[MAX_KEYS];

bool Inputs::isKeyHeld(KeyCode key) {
  return currentKeyState[(uint16_t) key];
}

bool Inputs::isKeyPressed(KeyCode key) {
  return currentKeyState[(uint16_t) key] && !previousKeyState[(uint16_t) key];
}

bool Inputs::isKeyReleased(KeyCode key) {
  return !currentKeyState[(uint16_t) key] && previousKeyState[(uint16_t) key];
}

void Inputs::update() {
  for (int i = 0; i < MAX_KEYS; i++) {
    previousKeyState[i] = currentKeyState[i];
  }
}

void GlfwInputs::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (key < 0 || key >= MAX_KEYS) {
    return; // Ignore out-of-bounds keys
  }

  if (action == GLFW_PRESS) {
    currentKeyState[key] = true;
  } else if (action == GLFW_RELEASE) {
    currentKeyState[key] = false;
  }
}

} // bird