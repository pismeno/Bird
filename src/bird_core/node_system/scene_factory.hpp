#pragma once

#include "node_system/scene.hpp"
#include "node_system/inode_manager.hpp"

#include <memory>
#include <functional>
#include <cassert>

#include "node_system/node_types.hpp"
#include "utils/result.hpp"

namespace bird::node_system {

using ManagerFactory = std::function<std::unique_ptr<INodeManager>()>;

/**
 * @brief Factory class for creating scenes.
 */
class SceneFactory {
 public:
  explicit SceneFactory() = default;

  /**
   * @brief Loads scene definitions from a json file, into this instance of SceneFactory.
   * @param path Path to the json file.
   * @return Result of the operation.
   */
  Result<void> load_scene_definitions_from(const std::string& path);

  /**
 * @brief Loads node definitions from a json file, into this instance of SceneFactory.
 * @param path Path to the json file.
 * @return Result of the operation.
 */
  Result<void> load_node_definitions_from(const std::string& path);

  /**
   * @brief Creates a scene of the given type, this method looks up scene types in this instance of SceneFactory.
   * @param scene_type the string id of the scene type to create.
   * @return created scene.
   */
  std::unique_ptr<Scene> create_scene(const std::string& scene_type) const;
  Result<std::unique_ptr<Scene>> load_scene_from(const std::string& path) const;
  Result<void> save_scene_to(const Scene& scene, const std::string& path) const;

  /**
 * @brief Registers a manager for this SceneFactory instance.
 * @note Asserts that the manager name must be unique.
 */
  template <typename T>
  void register_manager() {
    std::string name = T::MANAGER_NAME;

    assert(manager_registry.find(name) == manager_registry.end() && "Manager name already registered!");

    manager_registry[name] = []() -> std::unique_ptr<INodeManager> { return std::make_unique<T>(); };
  }

 private:
  struct NodeDefinition {
    std::vector<std::string> managers;
  };

  struct SceneDefinition {
    std::vector<std::string> managers;
    std::vector<std::string> allowed_nodes;
  };

  std::unordered_map<std::string, ManagerFactory> manager_registry;
  std::unordered_map<std::string, NodeDefinition> node_definitions;
  std::unordered_map<std::string, SceneDefinition> scene_definitions;
};

} // bird::node_system