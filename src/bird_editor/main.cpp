#include <iostream>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "windowing/iwindow.hpp"
#include "rendering/renderer.hpp"

int main() {
    using namespace bird;

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    std::unique_ptr<IWindow> window =
        IWindow::create(WindowOptions{800, 600, "Bird Editor"});

    if (!window->init()) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    Renderer renderer;
    renderer.create_renderer(bird::RendererType::Metal);
    renderer.init(*window);


    while (!window->shouldClose()) {
        window->update();
        renderer.render();

        // Window/input handling goes here.
        // Example:
        // auto& inputs = window->getInputs();

        if (window->isFullscreen()) {
            // Optional fullscreen logic
        }
    }

    window->close();

    glfwTerminate();
    return 0;
}