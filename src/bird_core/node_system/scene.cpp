#include "scene.hpp"

namespace bird::node_system {

[[nodiscard]] Node* Scene::getNode(NodeID id) {
  {
    auto it = nodes.find(id);
    return (it != nodes.end()) ? it->second.get() : nullptr;
  }
}

Result Scene::set_node_position(NodeID node_id, glm::vec2 pos) {
  Transform2D* transform = transform_system.get_transform(node_id);

  if (!transform) {
    return Result::fail("Transform not found, node probably doesnt have a transform");
  }

  transform->local_position = pos;
  transform->is_dirty = true;

  mark_spatial_children_dirty(node_id);
  return Result::ok();
}

Result Scene::mark_spatial_children_dirty(NodeID parent_id) {
  Node* parent = getNode(parent_id);
  if (!parent) {
    return Result::fail("Node with provided ID not found");
  }

  internal_mark_spatial_children_dirty(parent);

  return Result::ok();
}

// --- Internal Worker (The Speed Demon) ---
void Scene::internal_mark_spatial_children_dirty(Node* parent) {
  for (NodeID child_id : parent->childrenIds) {
    Node* child = getNode(child_id);
    if (!child) continue;

    Transform2D* child_transform = transform_system.get_transform(child_id);

    if (child_transform) {
      child_transform->is_dirty = true;
    }

    internal_mark_spatial_children_dirty(child);
  }
}

} // bird::node_system