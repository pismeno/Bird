#include <iostream>
#include <memory>

#include <GLFW/glfw3.h>

#include "../bird_core/rendering/renderer.hpp"


#include "../bird_core/windowing/window.hpp"
#include "../bird_core/node_system/managers/transform/transforms.hpp"

int main() {

  using namespace bird;
  using namespace bird::inputs;
  using namespace bird::node_system::managers;

  if (!glfwInit()) {
        // Handle initialization failure
        return -1;
  }

  std::unique_ptr<Window> window = Window::create(WindowOptions{600, 600, "Bird Editor"});
  window->init();

  Renderer renderer;
  renderer.create_renderer(RendererType::Metal);
  renderer.init(*window);
  window->setResizeCallback([&renderer](int width, int height) {
    renderer.handle_window_resize(width, height);
  });

  std::cout << "Hello, Bird Editor!" << std::endl;

  Transform2D quad_transform;
  quad_transform.local_position = {0.7f, -0.9f};
  quad_transform.local_scale = {1.0f, 1.0f};
  quad_transform.local_rotation = 23.0f;


  while (!window->shouldClose())
  {
    window->update();
    renderer.submit_quad(quad_transform);
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