#include <iostream>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "window.hpp"

int main() {

  using namespace bird;
  using namespace bird::inputs;

  if (!glfwInit()) {
        // Handle initialization failure
        return -1;
  }

  std::unique_ptr<Window> window = Window::create(800, 600, "Bird Editor");
  std::unique_ptr<Window> window2 = Window::create(800, 600, "Bird Editor 2");
  window->init();
  window2->init();

  std::cout << "Hello, Bird Editor!" << std::endl;

  while (!window->shouldClose())
  {
    window->update();
    window2->update();

    if (window->getInputs().isKeyPressed(KeyCode::Space)) {
      std::cout << "Space is pressed!" << std::endl;
    }

    if (window2->getInputs().isKeyPressed(KeyCode::Space)) {
      std::cout << "Space is pressed in window 2!" << std::endl;
    }

    if (window->getInputs().isMouseButtonReleased(MouseCode::Left)) {
      std::cout << "Left mouse button is released!" << std::endl;
    }

    if (window->getInputs().isKeyPressed(KeyCode::F11)) {
      window->setFullscreen(!window->isFullscreen());
    }

    window->getInputs().endFrame();
    window2->getInputs().endFrame();
  }

  window->close();

  glfwTerminate();
}