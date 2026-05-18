#include "scene_factory.hpp"
#include "node_system/scene.hpp"
#include "node_system/inode_manager.hpp"

#include <memory>
#include <functional>

#include "utils/result.hpp"

namespace bird::node_system {

std::unique_ptr<Scene> SceneFactory::create_scene(const std::string& scene_type) const {
  auto scene_def_it = scene_definitions.find(scene_type);

  if (scene_def_it == scene_definitions.end()) {
    return nullptr; // no scene with that name is registered
  }

  auto scene = std::make_unique<Scene>();
  SceneDefinition scene_definition = scene_def_it->second;

  for (auto& manager_name : scene_definition.managers) {
    auto manager_factory_it = manager_registry.find(manager_name);

    if (manager_factory_it == manager_registry.end()) {
      continue; // Skip the manager if it's not registered
    }

    ManagerFactory manager_factory = manager_factory_it->second;
    scene->managers.emplace(manager_name, manager_factory());
  }

  for (auto& node_name : scene_definition.allowed_nodes) {
    auto node_def_it = node_definitions.find(node_name);

    if (node_def_it == node_definitions.end()) {
      continue; // Skip the node if it's not registered
    }

    scene->node_types.emplace(node_def_it->first, node_def_it->second);
  }

  return scene;
}

Result SceneFactory::load_scene_definitions_from() {
  scene_definitions.emplace(
    "scene_2d",
    SceneDefinition{
        {"transform_manager", "drawable_manager"},
        {"node", "node_2d", "sprite_2d"}
    }
  );

  return Result::ok();
}

Result SceneFactory::load_node_definitions_from() {
  node_definitions.emplace(
      "node",
      NodeDefinition{
          {}
      }
  );

  node_definitions.emplace(
      "node_2d",
      NodeDefinition{
          {"transform_manager"}
      }
  );

  node_definitions.emplace(
      "sprite_2d",
      NodeDefinition{
          {"transform_manager", "drawable_manager"}
      }
  );

  return Result::ok();
}

} // bird::node_system