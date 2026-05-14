#include "node_system/render_gatherer.hpp"

#include <vector>

#include <glm/glm.hpp>

#include "node_system/scene.hpp"
#include "node_system/managers/drawable_manager.hpp"
#include "node_system/managers/transform_manager.hpp"
#include "rendering/render_command.hpp"

namespace bird::node_system {

using namespace managers;

std::vector<RenderCommand> RenderGatherer::gather_render_commands(Scene& scene) {
  auto drawable_manager = scene.get_manager<DrawableManager>();
  auto transform_manager = scene.get_manager<TransformManager>();

  std::vector<RenderCommand> commands;

  for (auto& drawable : drawable_manager->get_drawables()) {
    glm::mat3x3 transform = transform_manager->get_global_matrix(drawable.node_id);

    commands.push_back({
      transform,
      //drawable.texture_id,
      drawable.z_index
    });
  }

  return commands;
}

} // bird::node_system