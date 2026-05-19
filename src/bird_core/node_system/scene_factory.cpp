#include "scene_factory.hpp"
#include "node_system/scene.hpp"
#include "node_system/inode_manager.hpp"

#include <memory>
#include <functional>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "utils/result.hpp"

namespace bird::node_system {

using json = nlohmann::json;

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

Result SceneFactory::load_scene_definitions_from(const std::string& path) {
  scene_definitions.emplace(
    "scene_2d",
    SceneDefinition{
        {"transform_manager", "drawable_manager"},
        {"node", "node_2d", "sprite_2d"}
    }
  );

  return Result::ok();
}

Result SceneFactory::load_node_definitions_from(const std::string& path) {
  std::ifstream file(path);

  if (!file.is_open()) {
    return Result::fail("Failed to open '" + path + "'.");
  }

  json data = json::parse(file, nullptr, false);

  if (data.is_discarded()) {
    return Result::fail("Failed to parse JSON in '" + path + "'.");
  }

  if (!data.is_array()) {
    return Result::fail("Expected an array of node definitions in '" + path + "'.");
  }

  for (const auto& item : data) {
    if (!item.contains("name")) {
      return Result::fail("Node definition is missing 'name' property in '" + path + "'.");
    }

    NodeDefinition node_definition;

    if (item.contains("managers")) {
      if (!item["managers"].is_array()) {
        return Result::fail("Node definition 'managers' property is not an array in '" + path + "'.");
      }

      node_definition.managers = item["managers"].get<std::vector<std::string>>();
    }

    node_definitions.emplace(item["name"].get<std::string>(), std::move(node_definition));
  }

  return Result::ok();
}

} // bird::node_system