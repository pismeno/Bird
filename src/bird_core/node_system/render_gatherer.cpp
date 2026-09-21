#include <node_system/render_gatherer.hpp>

#include <vector>

#include <glm/glm.hpp>

#include <node_system/scene.hpp>
#include <node_system/managers/drawable_manager.hpp>
#include <node_system/managers/transform_manager.hpp>
#include <resources/render_command.hpp>

namespace bird::node_system {

using namespace managers;

std::vector<RenderCommand> RenderGatherer::gather_render_commands(Scene& scene) {
  auto drawable_manager = scene.get_manager<DrawableManager>();
  auto transform_manager = scene.get_manager<TransformManager>();

  if (!drawable_manager || !transform_manager) return {};

  const auto& matrices = transform_manager->get_all_matrices();
  const auto& drawables = drawable_manager->get_all_drawables();

  uint32_t max_index = scene.get_highest_allocated_node_index();

  std::vector<RenderCommand> commands;
  commands.reserve(max_index);

  for (uint32_t i = 0; i < max_index; ++i) {
    if (drawables[i].is_visible) {

      commands.emplace_back(RenderCommand{
          matrices[i],
          //drawables[i].texture_id,
          drawables[i].z_index
      });
    }
  }

  return commands;
}

} // bird::node_system