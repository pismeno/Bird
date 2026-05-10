#include <iostream>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

int main() {
    if (!glfwInit()) {
        // Handle initialization failure
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1280, 720, "My Title", NULL, NULL);

    if (!window) {
        std::cerr << "Failed to create window!" << std::endl;
        glfwTerminate();
        return -1;
    }



    std::cout << "Hello, Bird Editor!" << std::endl;

    while (!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}