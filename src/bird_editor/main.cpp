#include <iostream>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "window.hpp"
#include "../bird_core/inputs/inputs.hpp"
#include "../bird_core/inputs/input_codes.hpp"

int main() {

  using namespace bird::inputs;
  using namespace bird::editor;

  if (!glfwInit()) {
        // Handle initialization failure
        return -1;
  }

  std::unique_ptr<Window> window = Window::create(800, 600, "Bird Editor");
  window->init();

  std::cout << "Hello, Bird Editor!" << std::endl;

  while (!window->shouldClose())
  {
    window->update();
    if (Inputs::isKeyPressed(KeyCode::Space)) {
      std::cout << "Space is pressed!" << std::endl;
    }

    if (Inputs::isMouseButtonReleased(MouseCode::Left)) {
      std::cout << "Left mouse button is released!" << std::endl;
    }

    if (Inputs::isKeyPressed(KeyCode::F11)) {
      window->setFullscreen(!window->isFullscreen());
    }

    Inputs::endFrame();
  }

  window->close();

  glfwTerminate();
}