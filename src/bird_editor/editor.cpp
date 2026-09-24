#include <editor/editor.hpp>

#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <osal/iwindow.hpp>
#include <rendering/irenderer.hpp>
#include <glslang/Public/ShaderLang.h>

#include <osal/ifile_system.hpp>
#include <resources/asset_manager.hpp>
#include <node_system/scene_factory.hpp>

namespace bird {

Result<void> Editor::init() {
  auto r_create_file_system = IFileSystem::create();
  if (!r_create_file_system) {
    return bird::fail(r_create_file_system.error());
  }
  file_system = std::move(r_create_file_system).value();

  scene_factory = std::make_unique<SceneFactory>();

  glslang::InitializeProcess();

  if (!glfwInit()) {
    return bird::fail("Failed to initialize GLFW");
  }

  auto window_result = IWindow::create(WindowOptions{800, 600, "Bird Editor"});
  if (!window_result) {
    return bird::fail(window_result.error());
  }

  window = std::move(window_result).value();

  if (!window->init()) {
    glfwTerminate();
    return bird::fail("Failed to create window");
  }

  return bird::ok();
}

Result<void> Editor::run() {
  while (!window->should_close()) {
    window->update();
    renderer->render();

    if (renderer->needs_framebuffer_resize()) {
      auto r_resize_framebuffer = renderer->resize_framebuffer(
          window->get_framebuffer_width(),
          window->get_framebuffer_height()
      );

      if (!r_resize_framebuffer) {
        return bird::fail(r_resize_framebuffer.error());
      }
    }

    if (renderer->is_context_lost()) {
      return bird::fail("Renderer context lost");
    }
  }

  return bird::ok();
}

Result<void> Editor::shutdown() {
  // During shutdown, log errors but continue the teardown process
  // to prevent memory/resource leaks.
  if (renderer) {
    auto r_shutdown_result = renderer->shutdown();
    if (!r_shutdown_result) {
      std::cerr << "Error shutting down renderer: " << r_shutdown_result.error() << std::endl;
    }
    renderer.reset();
  }

  if (window) {
    auto window_close_result = window->close();
    if (!window_close_result) {
      std::cerr << "Error closing window: " << window_close_result.error() << std::endl;
    }
    window.reset();
  }

  glfwTerminate();
  glslang::FinalizeProcess();

  return bird::ok();
}

Result<void> Editor::make_context_current() {
  os_context = {
      .file_system = file_system.get()
  };

  asset_manager = std::make_unique<AssetManager>();

  resources_context = {
      .os_context = &os_context,
      .asset_manager = asset_manager.get()
  };

  auto r_init_am = asset_manager->init(resources_context);
  if (!r_init_am) {
    return r_init_am;
  }

  node_system_context = {
    .resources_context = &resources_context,
    .scene_factory = scene_factory.get()
  };

  rendering_context = {
    .resources_context = &resources_context
  };

  auto renderer_result = IRenderer::create(bird::RendererType::Vulkan, &rendering_context);
  if (!renderer_result) {
    return bird::fail(renderer_result.error());
  }

  renderer = std::move(renderer_result).value();

  auto r_attach_window_result = renderer->attach_window(
      window->get_native_handle(),
      window->get_framebuffer_width(),
      window->get_framebuffer_height()
  );

  if (!r_attach_window_result) {
    return bird::fail(r_attach_window_result.error());
  }

  auto r_init_result = renderer->init();
  if (!r_init_result) {
    return bird::fail(r_init_result.error());
  }

  context = {
      .node_system_context = &node_system_context,
      .rendering_context = &rendering_context,
      .os_context = &os_context,
      .resources_context = &resources_context
  };

  return bird::ok();
}

} // namespace bird