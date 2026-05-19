#include "transform_manager.hpp"

#include <algorithm>
#include <glm/glm.hpp>

namespace bird::node_system::managers {

Result TransformManager::set_node_position(NodeID node_id, glm::vec2 pos) {
  TransformInfo* transform = get_transform(node_id);

  if (!transform) {
    return Result::fail("Transform not found");
  }

  transform->local_position = pos;
  mark_spatial_children_dirty(node_id);

  return Result::ok();
}

Result TransformManager::set_node_scale(NodeID node_id, glm::vec2 scale) {
  TransformInfo* transform = get_transform(node_id);

  if (!transform) {
    return Result::fail("Transform not found");
  }

  transform->local_scale = scale;
  mark_spatial_children_dirty(node_id);

  return Result::ok();
}

Result TransformManager::set_node_shear(NodeID node_id, glm::vec2 shear) {
  TransformInfo* transform = get_transform(node_id);

  if (!transform) {
    return Result::fail("Transform not found");
  }

  transform->local_shear = shear;
  mark_spatial_children_dirty(node_id);

  return Result::ok();
}

Result TransformManager::set_node_rotation(NodeID node_id, float rotation) {
  TransformInfo* transform = get_transform(node_id);

  if (!transform) {
    return Result::fail("Transform not found");
  }

  transform->local_rotation = rotation;
  mark_spatial_children_dirty(node_id);

  return Result::ok();
}

void TransformManager::mark_spatial_children_dirty(NodeID parent_id) {
  TransformInfo* parent = get_transform(parent_id);

  if (!parent || parent->is_dirty) return;

  parent->is_dirty = true;
  NodeID current_child_id = parent->first_child_id;

  while (current_child_id != INVALID_NODE_ID) {
    TransformInfo* child = get_transform(current_child_id);
    if (child) {
      // Recursive call for grandchildren
      mark_spatial_children_dirty(current_child_id);

      // Move sideways to the next sibling
      current_child_id = child->next_sibling_id;
    } else {
      break; // Hit a Ghost or invalid ID
    }
  }
}

bool TransformManager::has_transform(NodeID node_id) const {
  if (node_id.index() >= node_to_transform.size()) return false;
  return node_to_transform[node_id.index()] != INVALID_TRANSFORM_INDEX;
}

TransformManager::TransformInfo* TransformManager::get_transform(NodeID node_id) {
  if (node_id.index() >= node_to_transform.size()) {
    return nullptr;
  }

  TransformIndex index = node_to_transform[node_id.index()];
  if (index == INVALID_TRANSFORM_INDEX || index >= transforms.size()) {
    return nullptr;
  }

  return &transforms[index];
}

void TransformManager::on_update() {
  for (size_t i = 0; i < transforms.size(); i++) {
    if (transforms[i].parent_id == INVALID_NODE_ID && transforms[i].node_id != INVALID_NODE_ID) {
      update_node_hierarchy(transforms[i].node_id, glm::mat3(1.0f));
    }
  }
}

void TransformManager::on_node_require_manager(NodeID node_id) {
  transforms.emplace_back();
  transforms.back().node_id = node_id;

  global_matrices[node_id.index()] = glm::mat3(1.0f);

  node_to_transform[node_id.index()] = transforms.size() - 1;
}

void TransformManager::update_node_hierarchy(NodeID node_id, const glm::mat3& parent_global_mat) {
  TransformInfo* transform = get_transform(node_id);
  if (!transform) return;

  uint32_t idx = node_id.index();
  glm::mat3 my_global_mat;

  if (transform->is_dirty) {
    float cos = std::cos(transform->local_rotation);
    float sin = std::sin(transform->local_rotation);

    float sx = transform->local_scale.x;
    float sy = transform->local_scale.y;
    float shx = transform->local_shear.x;
    float shy = transform->local_shear.y;

    glm::mat3 local_mat(
        sx * (cos - sin * shy), sx * (sin + cos * shy), 0.0f,
        sy * (cos * shx - sin), sy * (sin * shx + cos), 0.0f,
        transform->local_position.x, transform->local_position.y, 1.0f
    );

    my_global_mat = parent_global_mat * local_mat;

    global_matrices[idx] = my_global_mat;
    transform->is_dirty = false;
  } else {
    my_global_mat = global_matrices[idx];
  }

  // Recurse to children
  NodeID child_id = transform->first_child_id;
  while (child_id != INVALID_NODE_ID) {
    update_node_hierarchy(child_id, my_global_mat);
    TransformInfo* child_transform = get_transform(child_id);
    child_id = child_transform ? child_transform->next_sibling_id : INVALID_NODE_ID;
  }
}

void TransformManager::on_node_destroyed(NodeID node_id) {
  if (!has_transform(node_id)) return;

  remove_from_parent(node_id);

  TransformIndex index = node_to_transform[node_id.index()];

  // Invalidate the memory (Ghost ID)
  transforms[index].node_id = INVALID_NODE_ID;

  // Break the lookup link immediately
  node_to_transform[node_id.index()] = INVALID_TRANSFORM_INDEX;
}

void TransformManager::on_frame_end() { // In this hook we compact the TransformInfo array
  size_t write_idx = 0;
  for (size_t read_idx = 0; read_idx < transforms.size(); ++read_idx) {
    if (transforms[read_idx].node_id != INVALID_NODE_ID) {
      if (write_idx != read_idx) {
        transforms[write_idx] = transforms[read_idx];
        // REMOVED: global_matrices[write_idx] = global_matrices[read_idx];
      }
      write_idx++;
    }
  }

  // Resize the dense array to chop off the dead data
  transforms.resize(write_idx);

  // Rebuild the Lookup Table
  std::fill(node_to_transform.begin(), node_to_transform.end(), INVALID_TRANSFORM_INDEX);
  for (size_t i = 0; i < transforms.size(); ++i) {
    node_to_transform[transforms[i].node_id.index()] = i;
  }
}

void TransformManager::remove_from_parent(NodeID child_id) {
  TransformInfo* child = get_transform(child_id);
  if (!child || child->parent_id == INVALID_NODE_ID) return;

  TransformInfo* parent = get_transform(child->parent_id);
  if (!parent) return;

  // If this child is the first child, just point the parent to the next sibling
  if (parent->first_child_id == child_id) {
    parent->first_child_id = child->next_sibling_id;
  } else {
    // Otherwise, find the sibling immediately BEFORE this child, and patch the link
    NodeID current_sibling_id = parent->first_child_id;
    while (current_sibling_id != INVALID_NODE_ID) {
      TransformInfo* sibling = get_transform(current_sibling_id);
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
  TransformInfo* child = get_transform(child_id);
  if (!child) return;

  // Unlink from current parent to avoid corrupting the list
  if (child->parent_id != INVALID_NODE_ID) {
    remove_from_parent(child_id);
  }

  // ink to new parent
  if (new_parent_id != INVALID_NODE_ID) {
    TransformInfo* parent = get_transform(new_parent_id);
    if (parent) {
      child->parent_id = new_parent_id;
      child->next_sibling_id = parent->first_child_id;
      parent->first_child_id = child_id;
    }
  }

  mark_spatial_children_dirty(child_id);
}

void TransformManager::on_node_reparented(NodeID node_id, NodeID new_parent_id, NodeID old_parent_id) {
  set_parent(node_id, new_parent_id);
}

} // bird::node_system::managers