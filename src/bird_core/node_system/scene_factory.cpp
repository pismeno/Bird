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

std::unique_ptr<Scene> SceneFactory::create_scene(const std::string& scene_type) const {
  auto scene_def_it = scene_definitions.find(scene_type);

  if (scene_def_it == scene_definitions.end()) {
    return nullptr; // no scene with that name is registered
  }

  auto scene = std::make_unique<Scene>(scene_type);
  SceneDefinition scene_definition = scene_def_it->second;

  for (auto& manager_name : scene_definition.managers) {
    auto manager_factory_it = manager_registry.find(manager_name);

    if (manager_factory_it == manager_registry.end()) {
      continue; // Skip the manager if it's not registered
    }

    ManagerFactory manager_factory = manager_factory_it->second;
    scene->add_manager(manager_factory());
  }

  for (auto& node_name : scene_definition.allowed_nodes) {
    auto node_def_it = node_definitions.find(node_name);

    if (node_def_it == node_definitions.end()) {
      continue; // Skip the node if it's not registered
    }

    scene->add_node_type(node_name, node_def_it->second.managers);
  }

  return scene;
}

Result<void> SceneFactory::load_scene_definitions_from(const std::string& path) {
  std::ifstream file(path);

  if (!file.is_open()) {
    return bird::fail("Failed to open '" + path + "'.");
  }

  nlohmann::json data = nlohmann::json::parse(file, nullptr, false);

  if (data.is_discarded()) {
    return bird::fail("Failed to parse JSON in '" + path + "'.");
  }

  if (!data.is_array()) {
    return bird::fail("Expected an array of scene definitions in '" + path + "'.");
  }

  for (const auto& item : data) {
    if (!item.contains("name")) {
      return bird::fail("Scene definition is missing 'name' property in '" + path + "'.");
    }

    SceneDefinition scene_definition;

    if (item.contains("managers")) {
      if (!item["managers"].is_array()) {
        return bird::fail("Scene definition 'managers' property is not an array in '" + path + "'.");
      }

      scene_definition.managers = item["managers"].get<std::vector<std::string>>();
    }

    if (item.contains("allowed_nodes")) {
      if (!item["allowed_nodes"].is_array()) {
        return bird::fail("Scene definition 'allowed_nodes' property is not an array in '" + path + "'.");
      }

      scene_definition.allowed_nodes = item["allowed_nodes"].get<std::vector<std::string>>();
    }

    scene_definitions.emplace(item["name"].get<std::string>(), std::move(scene_definition));
  }

  return bird::ok();
}

Result<void> SceneFactory::load_node_definitions_from(const std::string& path) {
  std::ifstream file(path);

  if (!file.is_open()) {
    return bird::fail("Failed to open '" + path + "'.");
  }

  nlohmann::json data = nlohmann::json::parse(file, nullptr, false);

  if (data.is_discarded()) {
    return bird::fail("Failed to parse JSON in '" + path + "'.");
  }

  if (!data.is_array()) {
    return bird::fail("Expected an array of node definitions in '" + path + "'.");
  }

  for (const auto& item : data) {
    if (!item.contains("name")) {
      return bird::fail("Node definition is missing 'name' property in '" + path + "'.");
    }

    NodeDefinition node_definition;

    if (item.contains("managers")) {
      if (!item["managers"].is_array()) {
        return bird::fail("Node definition 'managers' property is not an array in '" + path + "'.");
      }

      node_definition.managers = item["managers"].get<std::vector<std::string>>();
    }

    node_definitions.emplace(item["name"].get<std::string>(), std::move(node_definition));
  }

  return bird::ok();
}

Result<void> SceneFactory::save_scene_to(const Scene &scene, const std::string& path) const {
  nlohmann::json data;

  data["scene"] = nlohmann::json::object();
  data["scene"]["type"] = scene.get_scene_type();
  scene.serialize(data["scene"]);

  std::ofstream file(path);

  if (!file.is_open()) {
    return bird::fail("Failed to open '" + path + "'.");
  }

  file << data.dump(2);
  file.close();

  return bird::ok();
}

std::unique_ptr<Scene> SceneFactory::load_scene_from(const std::string& path) const {
  std::ifstream file(path);

  if (!file.is_open()) {
    return nullptr;
  }

  nlohmann::json data = nlohmann::json::parse(file, nullptr, false);
  if (data.is_discarded()) {
    return nullptr;
  }

  if (!data.contains("scene") || !data["scene"].is_object()) {
    return nullptr;
  }

  const auto& scene_json = data["scene"];

  if (!scene_json.contains("type") || !scene_json["type"].is_string()) {
    return nullptr;
  }

  auto scene = create_scene(scene_json["type"].get<std::string>());
  if (!scene) {
    return nullptr;
  }

  scene->deserialize(scene_json);

  return scene;
}

} // bird::node_system