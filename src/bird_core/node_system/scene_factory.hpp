#pragma once

#include "node_system/scene.hpp"
#include "node_system/inode_manager.hpp"

#include <memory>
#include <functional>

#include "node_system/node_types.hpp"
#include "utils/result.hpp"

namespace bird::node_system {

using ManagerFactory = std::function<std::unique_ptr<INodeManager>()>;

class SceneFactory {
 public:
  SceneFactory() = default;

  Result load_scene_definitions_from(const std::string& path);
  Result load_node_definitions_from(const std::string& path);

  std::unique_ptr<Scene> create_scene(const std::string& scene_type) const;
  //std::unique_ptr<Scene> load_scene_from();

  template <typename T>
  Result register_manager() {
    std::string name = T::MANAGER_NAME;

    if (manager_registry.find(name) != manager_registry.end()) {
      return Result::fail("a manager with the name '" + name + "' is already registered");
    }

    manager_registry[name] = []() -> std::unique_ptr<INodeManager> { return std::make_unique<T>(); };

    return Result::ok();
  }

 private:
  struct SceneDefinition {
    std::vector<std::string> managers;
    std::vector<std::string> allowed_nodes;
  };

  std::unordered_map<std::string, ManagerFactory> manager_registry;
  std::unordered_map<std::string, NodeDefinition> node_definitions;
  std::unordered_map<std::string, SceneDefinition> scene_definitions;
};

} // bird::node_system