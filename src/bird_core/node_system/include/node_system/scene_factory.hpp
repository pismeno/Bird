#pragma once

#include <node_system/scene.hpp>
#include <node_system/inode_manager.hpp>

#include <memory>
#include <functional>
#include <cassert>

#include <node_system/node_types.hpp>
#include <node_system/node_system_context.hpp>
#include <resources/asset_manager.hpp>
#include <osal/ifile_system.hpp>
#include <utils/result.hpp>

namespace bird {

using ManagerFactory = std::function<std::unique_ptr<INodeManager>()>;

/**
 * @brief Factory class for creating scenes.
 */
class SceneFactory {
 public:
  /**
   * @brief Loads scene definitions from a json file, into this instance of SceneFactory.
   * @param path Path to the json file.
   * @return Result of the operation.
   */
  Result<void> load_scene_definitions_from(const std::string_view definitions_source);

  /**
 * @brief Loads node definitions from a json file, into this instance of SceneFactory.
 * @param path Path to the json file.
 * @return Result of the operation.
 */
  Result<void> load_node_definitions_from(const std::string_view definitions_source);

  /**
   * @brief Creates a scene of the given type, this method looks up scene types in this instance of SceneFactory.
   * @param scene_type the string id of the scene type to create.
   * @return created scene.
   */
  Result<std::unique_ptr<Scene>> create_scene(const std::string& scene_type) const;
  Result<std::unique_ptr<Scene>> load_scene_from(const std::string_view scene_source) const;
  Result<void> save_scene_to(const Scene& scene, const std::string& path) const;

  /**
 * @brief Registers a manager for this SceneFactory instance.
 * @note Asserts that the manager name must be unique.
 */
  template <typename T>
  requires std::derived_from<T, INodeManager> && std::default_initializable<T>
  void register_manager() {
    std::string name = T::MANAGER_NAME;

    assert(manager_registry.find(name) == manager_registry.end() && "Manager name already registered!");

    manager_registry[name] = []() -> std::unique_ptr<INodeManager> { return std::make_unique<T>(); };
  }

  /**
   * @brief Creates a SceneFactory instance.
   */
  static Result<std::unique_ptr<SceneFactory>> create(const NodeSystemContext& context);
 private:
  explicit SceneFactory() = default;

  Result<void> init(const NodeSystemContext& context);

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

  AssetManager* asset_manager;
  IFileSystem* file_system;
};

} // bird