#include <iostream>
#include <algorithm>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "windowing/window.hpp"
#include "node_system/scene.hpp"
#include "node_system/node.hpp"
#include "node_system/managers/transform_manager.hpp"
#include "node_system/managers/drawable_manager.hpp"
#include "node_system/render_gatherer.hpp"
#include "rendering/render_command.hpp"

void debug_print_commands(const std::vector<RenderCommand>& commands) {
  std::cout << "--- Frame Render Commands (" << commands.size() << ") ---\n";
  for (size_t i = 0; i < commands.size(); ++i) {
    const auto& cmd = commands[i];

    std::cout << "[" << i << "] "
              << "TexID: " << cmd.texture_id
              << " | Z: " << cmd.z_index
              << " | mat: (" << glm::to_string(cmd.transform) << ")\n";
  }
  std::cout << "------------------------------------------" << std::endl;
}

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
  NodeID childID = scene->createNode2DDrawable(parID, "child 1");
  NodeID childID2 = scene->createNode2DDrawable(childID, "child 2");
  NodeID famID = scene->createNode2DDrawable(0, "fam parent");
  NodeID famChildID = scene->createNode2DDrawable(famID, "fam child");

  auto transform_manager = scene->get_manager<TransformManager>();
  auto drawable_manager = scene->get_manager<DrawableManager>();

  transform_manager->set_node_position(childID, glm::vec2(100, 100));
  transform_manager->set_node_position(childID2, glm::vec2(100, 100));
  transform_manager->set_node_position(famChildID, glm::vec2(400, 400));
  transform_manager->set_node_position(famID, glm::vec2(400, 400));

  std::cout << "Parent node ID: " << parID << std::endl;
  std::cout << "Parent node name: " << scene->getNode(parID)->getName() << std::endl;
  std::cout << "Child node ID: " << childID << std::endl;
  std::cout << "Child node name: " << scene->getNode(childID)->getName() << std::endl;
  std::cout << "Child node parent name: " << scene->getNode(scene->getNode(childID)->getParentId())->getName() << std::endl;
  std::cout << "Node's z-index: " << drawable_manager->get_drawable(famChildID)->z_index << std::endl;

  RenderGatherer gatherer;

  while (!window->shouldClose())
  {
    window->update();
    window2->update();

    if (window->getInputs().isKeyPressed(KeyCode::Space)) {
      std::cout << "Space is pressed!" << std::endl;

      transform_manager->set_node_position(famID, glm::vec2(50, 50));
    }

    if (window2->getInputs().isKeyPressed(KeyCode::Space)) {
      std::cout << "Space is pressed in window 2!" << std::endl;

      transform_manager->set_node_position(childID, glm::vec2(200, 100));
    }

    if (window->getInputs().isMouseButtonReleased(MouseCode::Left)) {
      std::cout << "Left mouse button is released!" << std::endl;

      auto commands = gatherer.gather_render_commands(*scene);
      debug_print_commands(commands);
    }

    if (window->getInputs().isKeyPressed(KeyCode::F11)) {
      window->setFullscreen(!window->isFullscreen());
    }

    scene->update();

    window->getInputs().endFrame();
    window2->getInputs().endFrame();
    scene->end_frame();
  }

  window->close();

  glfwTerminate();
}