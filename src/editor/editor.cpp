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
#include "resources/material_pool.hpp"

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

  material_pool = std::make_unique<MaterialPool>();
  resources_context.material_pool = material_pool.get();

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

  bindless_texture_registry = std::make_unique<BindlessTextureRegistry>();
  rendering_context.bindless_texture_registry = bindless_texture_registry.get();
  material_shader_layout_reflector = std::make_unique<MaterialShaderLayoutReflector>();
  rendering_context.material_shader_layout_reflector = material_shader_layout_reflector.get();
  material_shader_layout_registry = std::make_unique<MaterialShaderLayoutRegistry>(rendering_context);
  rendering_context.material_shader_layout_registry = material_shader_layout_registry.get();
  material_packer = std::make_unique<MaterialPacker>(rendering_context);
  rendering_context.material_packer = material_packer.get();
  packed_material_cache = std::make_unique<PackedMaterialCache>(rendering_context);
  rendering_context.packed_material_cache = packed_material_cache.get();

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

  auto r_s_c1 = shader_compiler->compile_glsl_file_to_spirv("core://shaders/mat_a.vert.glsl",
                                                            "core://shaders/mat_a.vert.spv");
  if (!r_s_c1) return bird::fail(r_s_c1.error());

  auto r_s_c2 = shader_compiler->compile_glsl_file_to_spirv("core://shaders/mat_a.frag.glsl",
                                                            "core://shaders/mat_a.frag.spv");
  if (!r_s_c2) return bird::fail(r_s_c2.error());

  auto r_s_c3 = shader_compiler->compile_glsl_file_to_spirv("core://shaders/mat_b.vert.glsl",
                                                            "core://shaders/mat_b.vert.spv");
  if (!r_s_c3) return bird::fail(r_s_c3.error());

  auto r_s_c4 = shader_compiler->compile_glsl_file_to_spirv("core://shaders/mat_b.frag.glsl",
                                                            "core://shaders/mat_b.frag.spv");
  if (!r_s_c4) return bird::fail(r_s_c4.error());

  auto r_get_texture = asset_manager->acquire<ImageAsset>("editor://test_img.png");
  if (!r_get_texture) return bird::fail(r_get_texture.error());
  AssetHandle<ImageAsset> texture = std::move(r_get_texture).value();
  std::cout << bindless_texture_registry->get_index(texture) << " - Tex index" << std::endl;;

  auto r_get_vert_shader_a = asset_manager->acquire<VertexShaderAsset>("core://shaders/mat_a.vert.spv");
  if (!r_get_vert_shader_a) return bird::fail(r_get_vert_shader_a.error());
  AssetHandle<VertexShaderAsset> vert_shader_a = std::move(r_get_vert_shader_a).value();
  auto r_get_frag_shader_a = asset_manager->acquire<FragmentShaderAsset>("core://shaders/mat_a.frag.spv");
  if (!r_get_frag_shader_a) return bird::fail(r_get_frag_shader_a.error());
  AssetHandle<FragmentShaderAsset> frag_shader_a = std::move(r_get_frag_shader_a).value();

  auto r_get_vert_shader = asset_manager->acquire<VertexShaderAsset>("core://shaders/mat_b.vert.spv");
  if (!r_get_vert_shader) return bird::fail(r_get_vert_shader.error());
  AssetHandle<VertexShaderAsset> vert_shader_b = std::move(r_get_vert_shader).value();
  auto r_get_frag_shader = asset_manager->acquire<FragmentShaderAsset>("core://shaders/mat_b.frag.spv");
  if (!r_get_frag_shader) return bird::fail(r_get_frag_shader.error());
  AssetHandle<FragmentShaderAsset> frag_shader_b = std::move(r_get_frag_shader).value();

  material_pool->add("test_mat_a", Material{
      .properties = {
          {"test_texture", texture}
          },
      .vertex_shader = std::move(vert_shader_a),
      .fragment_shader = std::move(frag_shader_a),
  });

  material_pool->add("test_mat_b", Material{
      .properties = {
          {"test_texture", texture},
          {"tint", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)}
      },
      .vertex_shader = std::move(vert_shader_b),
      .fragment_shader = std::move(frag_shader_b),
  });


  RendererOptions renderer_options{
      RendererType::Vulkan,
      window->get_native_handle(),
      window->get_framebuffer_width(),
      window->get_framebuffer_height()
  };

  auto renderer_result = IRenderer::create(renderer_options, rendering_context);
  if (!renderer_result) {
    return bird::fail(renderer_result.error());
  }

  renderer = std::move(renderer_result).value();
  rendering_context.renderer = renderer.get();

  return bird::ok();
}

Result<void> Editor::run() {
  MaterialID test_mat_id = MaterialID::from(0, 0);
  MaterialID test_mat_id2 = MaterialID::from(1, 0);

  while (!window->should_close()) {
    bird::ViewData view_data{};

    float width = static_cast<float>(window->get_framebuffer_width());
    float height = static_cast<float>(window->get_framebuffer_height());

    view_data.projection = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    view_data.view = glm::mat4(1.0f);
    view_data.viewProjection = view_data.projection * view_data.view;

    window->update();

    renderer->begin_frame();
    renderer->begin_pass(view_data);

    // --- TEST COMMANDS ---
    std::vector<RenderQuadCommand> test_commands;

    // Helper lambda to create a 2D affine transform matrix (Scale + Translation)
    auto make_transform = [](float x, float y, float w, float h) {
      glm::mat3 m(1.0f);
      m[0][0] = w; // Scale X
      m[1][1] = h; // Scale Y
      m[2][0] = x; // Translation X
      m[2][1] = y; // Translation Y
      return m;
    };

    // Quad 1: Large background quad
    RenderQuadCommand cmd1;
    cmd1.material_id = test_mat_id2;
    cmd1.z_index = 0;
    cmd1.transform = make_transform(100.0f, 100.0f, 400.0f, 400.0f);
    test_commands.push_back(cmd1);

    // Quad 2: Smaller quad, overlapping Quad 1 (higher Z-index draws on top)
    RenderQuadCommand cmd2;
    cmd2.material_id = test_mat_id;
    cmd2.z_index = 1;
    cmd2.transform = make_transform(300.0f, 300.0f, 200.0f, 200.0f);
    test_commands.push_back(cmd2);

    // Quad 3: Another quad, overlapping Quad 2 (highest Z-index)
    RenderQuadCommand cmd3;
    cmd3.material_id = test_mat_id;
    cmd3.z_index = 2;
    cmd3.transform = make_transform(200.0f, 400.0f, 150.0f, 150.0f);
    test_commands.push_back(cmd3);

    // Submit the commands to the queue
    renderer->submit(test_commands);
    // ---------------------

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