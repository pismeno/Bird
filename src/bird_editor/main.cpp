#include <iostream>
#include <memory>

#include <GLFW/glfw3.h>

#include "../bird_core/rendering/renderer.hpp"


#include "window.hpp"
#include "nodes/node_manager.hpp"

int main() {

  using namespace bird;
  using namespace bird::inputs;
  using namespace bird::nodes;

  if (!glfwInit()) {
        // Handle initialization failure
        return -1;
  }

  std::unique_ptr<Window> window = Window::create(WindowOptions{800, 600, "Bird Editor"});
  window->init();

  Renderer renderer;
  renderer.create_renderer(RendererType::Metal);
  renderer.init(*window);
  window->setResizeCallback([&renderer](int width, int height) {
    renderer.handle_window_resize(width, height);
  });

  std::cout << "Hello, Bird Editor!" << std::endl;

  std::unique_ptr<NodeManager> nodeManager = std::make_unique<NodeManager>();
  NodeID rootID = nodeManager->createNode<Node>(0);

  std::cout << "Root node ID: " << rootID << std::endl;

  while (!window->shouldClose())
  {
    window->update();
    renderer.render();

    if (window->getInputs().isKeyPressed(KeyCode::Space)) {
      std::cout << "Space is pressed!" << std::endl;
    }


    if (window->getInputs().isMouseButtonReleased(MouseCode::Left)) {
      std::cout << "Left mouse button is released!" << std::endl;
    }

    if (window->getInputs().isKeyPressed(KeyCode::F11)) {
      window->setFullscreen(!window->isFullscreen());
    }

    window->getInputs().endFrame();
  }

  renderer.shutdown();
  window->close();

  glfwTerminate();
}