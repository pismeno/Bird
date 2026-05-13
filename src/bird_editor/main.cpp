#include <iostream>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "window.hpp"
#include "nodes/scene.hpp"
#include "nodes/node.hpp"
#include "nodes/node_2d.hpp"

int main() {

  using namespace bird;
  using namespace bird::inputs;
  using namespace bird::nodes;

  if (!glfwInit()) {
        // Handle initialization failure
        return -1;
  }

  std::unique_ptr<Window> window = Window::create(WindowOptions{800, 600, "Bird Editor"});
  std::unique_ptr<Window> window2 = Window::create(WindowOptions{800, 600, "Bird Editor 2"});
  window->init();
  window2->init();

  std::cout << "Hello, Bird Editor!" << std::endl;

  std::unique_ptr<Scene> scene = std::make_unique<Scene>();
  NodeID parID = scene->createNode<Node>(0, "parent");
  NodeID childID = scene->createNode<Node2D>(parID, "child 1");

  std::cout << "Parent node ID: " << parID << std::endl;
  std::cout << "Parent node name: " << scene->getNode(parID)->getName() << std::endl;
  std::cout << "Child node ID: " << childID << std::endl;
  std::cout << "Child node name: " << scene->getNode(childID)->getName() << std::endl;
  std::cout << "Child node parent name: " << scene->getNode(scene->getNode(childID)->getParentId())->getName() << std::endl;

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