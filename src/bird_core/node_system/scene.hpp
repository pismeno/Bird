#pragma once

#include "node_types.hpp"
#include "node.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <stdexcept>

#include "utils/result.hpp"
#include "glm/vec2.hpp"
#include "imanager.hpp"

namespace bird::node_system {

class Scene {
 public:
  Scene() = default;
  static std::unique_ptr<Scene> create();

  [[nodiscard]] NodeID createNode(NodeID parent_id, std::string name);
  [[nodiscard]] NodeID createNode2D(NodeID parent_id, std::string name);
  [[nodiscard]] NodeID createNode2DDrawable(NodeID parent_id, std::string name);

  [[nodiscard]] Node* getNode(NodeID node_id);
  Result destroy_node(NodeID node_id);
  Result reparent_node(NodeID node_id, NodeID new_parent_id);
  void update();
  void end_frame();

  template <typename T, typename... Args>
  T& add_manager(Args&&... args) {
    size_t id = IManager::get_type_id<T>();

    // Ensure the lookup array is big enough
    if (id < fast_lookup.size() && fast_lookup[id] != nullptr) {
      return *static_cast<T*>(fast_lookup[id]);
    }

    if (id >= fast_lookup.size()) {
      fast_lookup.resize(id + 1);
    }

    // Create the manager
    auto manager = std::make_unique<T>(std::forward<Args>(args)...);
    T* raw_ptr = manager.get();

    // Store it
    fast_lookup[id] = raw_ptr;
    managers.push_back(std::move(manager));

    return *raw_ptr;
  }

  template <typename T>
  T* get_manager() {
    size_t id = IManager::get_type_id<T>();

    if (id >= fast_lookup.size() || !fast_lookup[id]) {
      return nullptr; // Doesn't exist
    }

    return static_cast<T*>(fast_lookup[id]);
  }

 private:
  std::atomic<NodeID> idCounter{0};
  std::unordered_map<NodeID, std::unique_ptr<Node>> nodes;

  std::vector<std::unique_ptr<IManager>> managers;

  std::vector<IManager*> fast_lookup;
};

} // bird::node_system