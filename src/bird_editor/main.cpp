#include <iostream>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "windowing/iwindow.hpp"
#include "rendering/irenderer.hpp"
#include "glslang/Public/ShaderLang.h"

int main() {
    using namespace bird;

  glslang::InitializeProcess();

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    auto window_result =
        IWindow::create(WindowOptions{800, 600, "Bird Editor"});

    if (!window_result) {
      std::cerr << window_result.error() << std::endl;
      return -1;
    }

    std::unique_ptr<IWindow> window = std::move(window_result).value();

    if (!window->init()) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    auto renderer_result = IRenderer::create(bird::RendererType::Vulkan);
    if (!renderer_result) {
        std::cerr << renderer_result.error() << std::endl;
        return -1;
    }

    std::unique_ptr<IRenderer> renderer = std::move(renderer_result).value();
    auto r_attach_window_result = renderer->attach_window(
        window->get_native_handle(),
        window->get_framebuffer_width(),
        window->get_framebuffer_height()
        );

    if (!r_attach_window_result) {
        std::cerr << r_attach_window_result.error() << std::endl;
        return -1;
    }

    auto r_init_result = renderer->init();
    if (!r_init_result) {
        std::cerr << r_init_result.error() << std::endl;
        return -1;
    }

    while (!window->should_close()) {
        window->update();
        renderer->render();
    }

    auto r_shutdown_result = renderer->shutdown();

    if (!r_shutdown_result) {
        std::cerr << r_shutdown_result.error() << std::endl;
        return -1;
    }

    auto window_close_result = window->close();
    if (!window_close_result) {
        std::cerr << window_close_result.error() << std::endl;
        return -1;
    }

    glfwTerminate();
  glslang::FinalizeProcess();
  return 0;
}