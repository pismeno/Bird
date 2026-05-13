#pragma once

#include "node_types.hpp"
#include "node.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <unordered_map>

#include "../utils/result.hpp"
#include "glm/vec2.hpp"
#include "managers/transform/transform_manager.hpp"

namespace bird::node_system {

using namespace managers;

class Scene {
 public:
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
        if (Transform2D* childTransform = transform_system.get_transform(newId)) {
          // Store parent ID as transform index (they use compatible numeric spaces)
          childTransform->spatial_parent_index = static_cast<TransformIndex>(parent_id);
        }
      }
    }

    return newId;
  }

  [[nodiscard]] Node* getNode(NodeID id);
  Result set_node_position(NodeID node_id, glm::vec2 pos);
  Result mark_spatial_children_dirty(NodeID parent_id);
  inline TransformManager& get_transform_system() { return transform_system; }

 private:
  std::atomic<NodeID> idCounter{0};
  std::unordered_map<NodeID, std::unique_ptr<Node>> nodes;

  TransformManager transform_system;

  void internal_mark_spatial_children_dirty(Node* parent);
};

} // bird::node_system