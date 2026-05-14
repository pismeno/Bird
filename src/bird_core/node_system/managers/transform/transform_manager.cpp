#include "transform_manager.hpp"
#include "transforms.hpp"

#include <algorithm>
#include <glm/glm.hpp>

namespace bird::node_system::managers {

void TransformManager::remove_from_parent(NodeID child_id) {
  Transform2D* child = get_transform(child_id);
  if (!child || child->parent_id == INVALID_NODE_ID) return;

  Transform2D* parent = get_transform(child->parent_id);
  if (!parent) return;

  // If this child is the first child, just point the parent to the next sibling
  if (parent->first_child_id == child_id) {
    parent->first_child_id = child->next_sibling_id;
  } else {
    // Otherwise, find the sibling immediately BEFORE this child, and patch the link
    NodeID current_sibling_id = parent->first_child_id;
    while (current_sibling_id != INVALID_NODE_ID) {
      Transform2D* sibling = get_transform(current_sibling_id);
      if (!sibling) break;

      if (sibling->next_sibling_id == child_id) {
        sibling->next_sibling_id = child->next_sibling_id; // Bypass the removed child
        break;
      }
      current_sibling_id = sibling->next_sibling_id;
    }
  }

  // Clear the child's links
  child->parent_id = INVALID_NODE_ID;
  child->next_sibling_id = INVALID_NODE_ID;
}

void TransformManager::set_parent(NodeID child_id, NodeID new_parent_id) {
  Transform2D* child = get_transform(child_id);
  if (!child) return;

  // 1. Unlink from current parent to avoid corrupting the list
  if (child->parent_id != INVALID_NODE_ID) {
    remove_from_parent(child_id);
  }

  // 2. Link to new parent
  if (new_parent_id != INVALID_NODE_ID) {
    Transform2D* parent = get_transform(new_parent_id);
    if (parent) {
      child->parent_id = new_parent_id;
      // Prepend to the parent's linked list
      child->next_sibling_id = parent->first_child_id;
      parent->first_child_id = child_id;
    }
  }

  // 3. Mark dirty since its global space just changed
  child->is_dirty = true;
  mark_spatial_children_dirty(child_id);
}

Result TransformManager::set_node_position(NodeID node_id, glm::vec2 pos) {
  Transform2D* transform = get_transform(node_id);

  if (!transform) {
    return Result::fail("Transform not found");
  }

  transform->local_position = pos;
  transform->is_dirty = true;

  mark_spatial_children_dirty(node_id);
  return Result::ok();
}

void TransformManager::mark_spatial_children_dirty(NodeID parent_id) {
  Transform2D* parent = get_transform(parent_id);
  if (!parent) return;

  // Jump strictly through the Intrusive Linked List
  NodeID current_child_id = parent->first_child_id;

  while (current_child_id != INVALID_NODE_ID) {
    Transform2D* child = get_transform(current_child_id);
    if (child) {
      child->is_dirty = true;

      // Recursive call for grandchildren
      mark_spatial_children_dirty(current_child_id);

      // Move sideways to the next sibling
      current_child_id = child->next_sibling_id;
    } else {
      break; // Hit a Ghost or invalid ID
    }
  }
}

void TransformManager::ensure_capacity(size_t capacity) {
  if (node_to_transform.size() < capacity) {
    node_to_transform.resize(capacity, INVALID_TRANSFORM_INDEX);
  }
}

void TransformManager::create_transform(NodeID node_id) {
  ensure_capacity(static_cast<size_t>(node_id) + 1);

  transforms.push_back(Transform2D());
  transforms.back().node_id = node_id;

  global_matrices.push_back(glm::mat3(1.0f));

  node_to_transform[static_cast<size_t>(node_id)] = static_cast<TransformIndex>(transforms.size() - 1);
}

bool TransformManager::has_transform(NodeID node_id) const {
  if (static_cast<size_t>(node_id) >= node_to_transform.size()) return false;
  return node_to_transform[static_cast<size_t>(node_id)] != INVALID_TRANSFORM_INDEX;
}

glm::mat3x3 TransformManager::get_global_matrix(NodeID node_id) {
  if (static_cast<size_t>(node_id) >= node_to_transform.size()) {
    return glm::mat3(1.0f);
  }

  TransformIndex index = node_to_transform[static_cast<size_t>(node_id)];
  if (index == INVALID_TRANSFORM_INDEX || static_cast<size_t>(index) >= global_matrices.size()) {
    return glm::mat3(1.0f);
  }

  return global_matrices[static_cast<size_t>(index)];
}

Transform2D* TransformManager::get_transform(NodeID node_id) {
  if (static_cast<size_t>(node_id) >= node_to_transform.size()) {
    return nullptr;
  }

  TransformIndex index = node_to_transform[static_cast<size_t>(node_id)];
  if (index == INVALID_TRANSFORM_INDEX || static_cast<size_t>(index) >= transforms.size()) {
    return nullptr;
  }

  return &transforms[static_cast<size_t>(index)];
}

void TransformManager::on_update() {
  for (size_t i = 0; i < transforms.size(); i++) {
    Transform2D& transform = transforms[i];

    if (transform.node_id == INVALID_NODE_ID) continue;

    if (transform.is_dirty) {

      // 1. Fast Manual 3x3 Matrix Construction (TRS)
      float cos = std::cos(transform.local_rotation);
      float sin = std::sin(transform.local_rotation);

      // GLM is Column-Major: mat3(col0, col1, col2)
      glm::mat3 local_mat(
          cos * transform.local_scale.x,  sin * transform.local_scale.x, 0.0f,
          -sin * transform.local_scale.y,  cos * transform.local_scale.y, 0.0f,
          transform.local_position.x,   transform.local_position.y,  1.0f
      );

      // 2. Hierarchical Composition
      if (transform.parent_id != INVALID_NODE_ID) {
        TransformIndex spatial_parent_id = INVALID_TRANSFORM_INDEX;
        if (static_cast<size_t>(transform.parent_id) < node_to_transform.size()) {
          spatial_parent_id = node_to_transform[static_cast<size_t>(transform.parent_id)];
        }

        if (spatial_parent_id != INVALID_TRANSFORM_INDEX && static_cast<size_t>(spatial_parent_id) < global_matrices.size()) {
          global_matrices[i] = global_matrices[static_cast<size_t>(spatial_parent_id)] * local_mat;
        } else {
          global_matrices[i] = local_mat; // parent has no transform or is invalid
        }
      } else {
        global_matrices[i] = local_mat;
      }

      // 3. Reset flag
      transform.is_dirty = false;
    }
  }
}

void TransformManager::on_node_destroyed(NodeID node_id) {
  if (!has_transform(node_id)) return;

  TransformIndex index = node_to_transform[static_cast<size_t>(node_id)];

  // 1. Invalidate the memory (Ghost ID)
  transforms[static_cast<size_t>(index)].node_id = INVALID_NODE_ID;

  // 2. Break the lookup link immediately
  node_to_transform[static_cast<size_t>(node_id)] = INVALID_TRANSFORM_INDEX;
}

void TransformManager::on_frame_end() {
  // 1. Physically remove all Ghost IDs from both flat arrays
  size_t write_idx = 0;
  for (size_t read_idx = 0; read_idx < transforms.size(); ++read_idx) {
    if (transforms[read_idx].node_id != INVALID_NODE_ID) {
      if (write_idx != read_idx) {
        transforms[write_idx] = std::move(transforms[read_idx]);
        global_matrices[write_idx] = std::move(global_matrices[read_idx]);
      }
      write_idx++;
    }
  }
  transforms.resize(write_idx);
  global_matrices.resize(write_idx);

  // 2. Rebuild the Lookup Table
  // Since elements shifted left, all indices in node_to_transform are now wrong.
  // We clear it and do one fast linear pass to restore it.
  std::fill(node_to_transform.begin(), node_to_transform.end(), INVALID_TRANSFORM_INDEX);

  for (size_t i = 0; i < transforms.size(); ++i) {
    node_to_transform[static_cast<size_t>(transforms[i].node_id)] = static_cast<TransformIndex>(i);
  }
}

} // bird::node_system::managers