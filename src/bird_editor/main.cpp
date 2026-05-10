#include <iostream>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "window.hpp"

int main() {

    using namespace bird::editor;

    if (!glfwInit()) {
        // Handle initialization failure
        return -1;
    }

    std::unique_ptr<Window> window = Window::create(800, 600, "Bird Editor");

    std::cout << "Hello, Bird Editor!" << std::endl;

    while (!window->shouldClose())
    {
        window->onUpdate();
    }
}