#include <editor/editor.hpp>

#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glslang/Public/ShaderLang.h>
#include <glm/gtc/matrix_transform.hpp>

#include <rendering/irenderer.hpp>
#include <osal/iwindow.hpp>
#include <osal/ifile_system.hpp>
#include <resources/asset_manager.hpp>
#include <node_system/scene_factory.hpp>
#include <node_system/managers/drawable_manager.hpp>
#include <node_system/managers/transform_manager.hpp>
#include <editor/shader_compiler.hpp>
#include <resources/asset_types.hpp>

namespace bird {

Result<void> Editor::init() {
  auto r_create_file_system = IFileSystem::create();
  if (!r_create_file_system) {
    return bird::fail(r_create_file_system.error());
  }
  file_system = std::move(r_create_file_system).value();

  file_system->mount("core", "../engine_data/core");
  file_system->mount("editor", "../engine_data/editor");

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
  };

  auto r_create_sf = SceneFactory::create(node_system_context);
  if (!r_create_sf) return bird::fail(r_create_sf.error());
  scene_factory = std::move(r_create_sf).value();
  node_system_context.scene_factory = scene_factory.get();

  scene_factory->register_manager<TransformManager>();
  scene_factory->register_manager<DrawableManager>();

  auto r_load_scene_defs = scene_factory->load_scene_definitions_from_file("editor://scene_definitions.json");
  if (!r_load_scene_defs) return bird::fail(r_load_scene_defs.error());
  auto r_load_node_defs = scene_factory->load_node_definitions_from_file("editor://node_definitions.json");
  if (!r_load_node_defs) return bird::fail(r_load_node_defs.error());

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

  auto r_s_c1 = shader_compiler->compile_glsl_file_to_spirv("core://shaders/forward_2d.vert.glsl",
                                                            "core://shaders/forward_2d.vert.spv");
  if (!r_s_c1) return bird::fail(r_s_c1.error());

  auto r_s_c2 = shader_compiler->compile_glsl_file_to_spirv("core://shaders/forward_2d.frag.glsl",
                                                            "core://shaders/forward_2d.frag.spv");
  if (!r_s_c2) return bird::fail(r_s_c2.error());

  auto r_s_c3 = shader_compiler->compile_glsl_file_to_spirv("core://shaders/triangle.vert.glsl",
                                                            "core://shaders/triangle.vert.spv");
  if (!r_s_c3) return bird::fail(r_s_c3.error());

  auto r_s_c4 = shader_compiler->compile_glsl_file_to_spirv("core://shaders/triangle.frag.glsl",
                                                            "core://shaders/triangle.frag.spv");
  if (!r_s_c4) return bird::fail(r_s_c4.error());

  auto r_s_c5 = shader_compiler->compile_glsl_file_to_spirv("core://shaders/vertices.vert.glsl",
                                                            "core://shaders/vertices.vert.spv");
  if (!r_s_c5) return bird::fail(r_s_c5.error());

  auto r_s_c6 = shader_compiler->compile_glsl_file_to_spirv("core://shaders/textures.frag.glsl",
                                                            "core://shaders/textures.frag.spv");
  if (!r_s_c6) return bird::fail(r_s_c6.error());

  auto r_load_img = asset_manager->acquire<ImageAsset>("editor://test_img.png");
  if (!r_load_img) return bird::fail(r_load_img.error());
  AssetHandle<ImageAsset> img = std::move(r_load_img).value();
  for (int i = 0; i < img->get_size_bytes(); i++) {
    std::cout << static_cast<int>(img->pixel_data[i]) << " ";
  }
  std::cout << std::endl;

  auto renderer_result = IRenderer::create(renderer_options, rendering_context);
  if (!renderer_result) {
    return bird::fail(renderer_result.error());
  }

  renderer = std::move(renderer_result).value();

  return bird::ok();
}

Result<void> Editor::run() {
  bool second = false;

  while (!window->should_close()) {
    bird::ViewData view_data{};

    view_data.view = glm::lookAt(
        glm::vec3(0.0f, -0.5f, 2.0f), // Eye
        glm::vec3(0.0f, -0.5f, 0.0f), // Target
        glm::vec3(0.0f, 1.0f, 0.0f)  // Up
    );
    if (second) {
      view_data.view = glm::lookAt(
          glm::vec3(0.0f, 0.5f, 2.0f), // Eye
          glm::vec3(0.0f, 0.5f, 0.0f), // Target
          glm::vec3(0.0f, 1.0f, 0.0f)  // Up
      );
      second = false;
    } else {
      second = true;
    }

    float aspect_ratio = static_cast<float>(window->get_framebuffer_width()) / static_cast<float>(window->get_framebuffer_height());

    view_data.projection = glm::perspective(
        glm::radians(45.0f),
        aspect_ratio,
        0.1f,
        10.0f
    );

    view_data.viewProjection = view_data.projection * view_data.view;

    window->update();
    renderer->begin_frame();
    renderer->begin_pass(view_data);
    renderer->end_pass();
    renderer->end_frame();

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