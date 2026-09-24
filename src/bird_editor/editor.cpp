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
#include "editor/shader_compiler.hpp"

namespace bird {

Result<void> Editor::init() {
  auto r_create_file_system = IFileSystem::create();
  if (!r_create_file_system) {
    return bird::fail(r_create_file_system.error());
  }
  file_system = std::move(r_create_file_system).value();

  file_system->mount("core", "../engine_data/core");
  file_system->mount("editor", "../engine_data/editor");

  scene_factory = std::make_unique<SceneFactory>();

  glslang::InitializeProcess();

  if (!glfwInit()) {
    return bird::fail("Failed to initialize GLFW");
  }

  auto window_result = IWindow::create(WindowOptions{800, 600, "Bird Editor"});
  if (!window_result) {
    glfwTerminate();
    return bird::fail(window_result.error());
  }
  window = std::move(window_result).value();

  os_context = {
      .file_system = file_system.get()
  };

  resources_context = {
      .os_context = &os_context,
  };

  auto r_create_am = AssetManager::create(resources_context);
  if (!r_create_am) {
    return bird::fail(r_create_am.error());
  }
  asset_manager = std::move(r_create_am).value();
  resources_context.asset_manager = asset_manager.get();

  node_system_context = {
      .resources_context = &resources_context,
      .scene_factory = scene_factory.get()
  };

  rendering_context = {
      .resources_context = &resources_context
  };

  RendererOptions renderer_options{
      RendererType::Vulkan,
      window->get_native_handle(),
      window->get_framebuffer_width(),
      window->get_framebuffer_height()
  };

  context = {
      .node_system_context = &node_system_context,
      .rendering_context = &rendering_context,
      .os_context = &os_context,
      .resources_context = &resources_context
  };

  auto r_create_sc = ShaderCompiler::create(context);
  if (!r_create_sc) return bird::fail(r_create_sc.error());

  shader_compiler = std::move(r_create_sc).value();

  context.shader_compiler = shader_compiler.get();

  auto r_s_c1 = shader_compiler->compile_glsl_to_spirv("core://shaders/forward_2d.vert.glsl", "core://shaders/forward_2d.vert.spv");
  if (!r_s_c1) return bird::fail(r_s_c1.error());
  auto r_s_c2 = shader_compiler->compile_glsl_to_spirv("core://shaders/forward_2d.frag.glsl", "core://shaders/forward_2d.frag.spv");
  if (!r_s_c2) return bird::fail(r_s_c2.error());
  auto r_s_c3 = shader_compiler->compile_glsl_to_spirv("core://shaders/triangle.vert.glsl", "core://shaders/triangle.vert.spv");
  if (!r_s_c3) return bird::fail(r_s_c3.error());
  auto r_s_c4 = shader_compiler->compile_glsl_to_spirv("core://shaders/triangle.frag.glsl", "core://shaders/triangle.frag.spv");
  if (!r_s_c4) return bird::fail(r_s_c4.error());

  auto renderer_result = IRenderer::create(renderer_options, rendering_context);
  if (!renderer_result) {
    return bird::fail(renderer_result.error());
  }

  renderer = std::move(renderer_result).value();

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

} // bird