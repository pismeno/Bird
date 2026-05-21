#pragma once

#include "node_types.hpp"

namespace bird::node_system {

class Scene;
class Node;

class NodeHandle {
 public:
  explicit NodeHandle() : scene(nullptr), node_id(INVALID_NODE_ID) {}
  NodeHandle(Scene* scene, NodeID node_id) : scene(scene), node_id(node_id) {}

  /**
   * @brief safely checks if the node still exists in the current generation.
   */
  [[nodiscard]] bool is_valid() const;

  explicit operator bool() const {
    return is_valid();
  }

  /**
   * @brief safely fetches the raw pointer. Returns nullptr if the node was destroyed.
   */
  [[nodiscard]] Node* get() const;

  Node* operator->() const { return get(); }
 private:
  Scene* scene;
  NodeID node_id;
};

} // bird::node_system