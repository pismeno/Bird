#include <iostream>
#include <algorithm>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "windowing/window.hpp"
#include "node_system/scene.hpp"
#include "node_system/node.hpp"
#include "node_system/managers/transform/transform_manager.hpp"

int main() {

  using namespace bird;
  using namespace bird::inputs;
  using namespace bird::node_system;
  using namespace bird::node_system::managers;

  if (!glfwInit()) {
        // Handle initialization failure
        return -1;
  }

  std::unique_ptr<Window> window = Window::create(WindowOptions{800, 600, "Bird Editor"});
  std::unique_ptr<Window> window2 = Window::create(WindowOptions{800, 600, "Bird Editor 2"});
  window->init();
  window2->init();

  std::cout << "Hello, Bird Editor!" << std::endl;

  std::unique_ptr<Scene> scene = Scene::create();
  NodeID parID = scene->createNode(0, "parent");
  NodeID childID = scene->createNode2D(parID, "child 1");
  NodeID childID2 = scene->createNode2D(childID, "child 2");

  auto transform_manager = scene->get_manager<TransformManager>();

  transform_manager->set_node_position(childID, glm::vec2(100, 100));
  transform_manager->set_node_position(childID2, glm::vec2(100, 100));

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

      transform_manager->set_node_position(childID, glm::vec2(200, 100));
    }

    if (window->getInputs().isMouseButtonReleased(MouseCode::Left)) {
      std::cout << "Left mouse button is released!" << std::endl;
    }

    if (window->getInputs().isKeyPressed(KeyCode::F11)) {
      window->setFullscreen(!window->isFullscreen());
    }

    scene->update();

    std::cout << "global mat 1: " << glm::to_string(transform_manager->get_transform(childID)->global_matrix) << std::endl;
    std::cout << "global mat 2: " << glm::to_string(transform_manager->get_transform(childID2)->global_matrix) << std::endl;

    window->getInputs().endFrame();
    window2->getInputs().endFrame();
    scene->end_frame();
  }

  window->close();

  glfwTerminate();
}