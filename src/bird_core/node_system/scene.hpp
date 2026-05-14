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
  static std::unique_ptr<Scene> create();

  template<typename T, typename... Args>
  [[nodiscard]] NodeID createNode(NodeID parent_id, Args&&... args) {
    NodeID newId = idCounter.fetch_add(1, std::memory_order_relaxed);

    auto node = std::make_unique<T>(std::forward<Args>(args)...);
    node->id = newId;
    node->parentId = INVALID_NODE_ID;

    node->on_enter_scene(*this);

    nodes[newId] = std::move(node);

    if (parent_id != INVALID_NODE_ID && parent_id != newId) {
      if (Node* parent = getNode(parent_id)) {
        parent->addChild(newId, *this);
        /*
        if (Transform2D* childTransform = transform_system.get_transform(newId)) {
          // Store parent ID as transform index (they use compatible numeric spaces)
          childTransform->spatial_parent_index = static_cast<TransformIndex>(parent_id);
        }
         */
      }
    }

    return newId;
  }

  [[nodiscard]] Node* getNode(NodeID id);
  Result destroy_node(NodeID id);

  template <typename T, typename... Args>
  T& add_manager(Args&&... args) {
    size_t id = IManager::get_type_id<T>();

    // Ensure the lookup array is big enough
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

  Scene() = default;

 private:

  std::atomic<NodeID> idCounter{0};
  std::unordered_map<NodeID, std::unique_ptr<Node>> nodes;

  std::vector<std::unique_ptr<IManager>> managers;

  std::vector<IManager*> fast_lookup;
};

} // bird::node_system