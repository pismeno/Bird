#include <node_system/managers/transform_manager.hpp>

#include <algorithm>
#include <glm/glm.hpp>

namespace bird::node_system::managers {

Result<void> TransformManager::set_node_position(NodeID node_id, glm::vec2 pos) {
  TransformInfo* transform = get_transform(node_id.index());

  if (!transform) {
    return bird::fail("Transform not found");
  }

  transform->local_position = pos;
  mark_spatial_children_dirty(node_id);

  return bird::ok();
}

Result<void> TransformManager::set_node_scale(NodeID node_id, glm::vec2 scale) {
  TransformInfo* transform = get_transform(node_id.index());

  if (!transform) {
    return bird::fail("Transform not found");
  }

  transform->local_scale = scale;
  mark_spatial_children_dirty(node_id);

  return bird::ok();
}

Result<void> TransformManager::set_node_shear(NodeID node_id, glm::vec2 shear) {
  TransformInfo* transform = get_transform(node_id.index());

  if (!transform) {
    return bird::fail("Transform not found");
  }

  transform->local_shear = shear;
  mark_spatial_children_dirty(node_id);

  return bird::ok();
}

Result<void> TransformManager::set_node_rotation(NodeID node_id, float rotation) {
  TransformInfo* transform = get_transform(node_id.index());

  if (!transform) {
    return bird::fail("Transform not found");
  }

  transform->local_rotation = rotation;
  mark_spatial_children_dirty(node_id);

  return bird::ok();
}

void TransformManager::mark_spatial_children_dirty(NodeID parent_id) {
  TransformInfo* parent = get_transform(parent_id.index());

  if (!parent || parent->is_dirty) return;

  parent->is_dirty = true;
  uint32_t current_child_idx = parent->first_child_idx;

  while (current_child_idx != INVALID_NODE_ID.index()) {
    TransformInfo* child = get_transform(current_child_idx);
    if (child) {
      // Recursive call for grandchildren
      mark_spatial_children_dirty(NodeID::from(current_child_idx, 0));

      // Move sideways to the next sibling
      current_child_idx = child->next_sibling_idx;
    } else {
      break; // Hit a Ghost or invalid ID
    }
  }
}

bool TransformManager::has_transform(NodeID node_id) const noexcept {
  if (node_id.index() >= node_to_transform.size()) return false;
  return node_to_transform[node_id.index()] != invalid_transform_index;
}

// Signature updated to match your new explicit requirement
TransformManager::TransformInfo* TransformManager::get_transform(uint32_t node_idx) {
  if (node_idx >= node_to_transform.size()) {
    return nullptr;
  }

  TransformIndex index = node_to_transform[node_idx];
  if (index == invalid_transform_index || index >= transforms.size()) {
    return nullptr;
  }

  return &transforms[index];
}

void TransformManager::on_update() noexcept {
  for (size_t i = 0; i < transforms.size(); i++) {
    if (transforms[i].parent_idx == INVALID_NODE_ID.index() && transforms[i].node_idx != INVALID_NODE_ID.index()) {
      update_node_hierarchy(NodeID::from(transforms[i].node_idx, 0), glm::mat3(1.0f));
    }
  }
}

void TransformManager::on_node_require_manager(NodeID node_id) noexcept {
  transforms.emplace_back();
  transforms.back().node_idx = node_id.index();

  global_matrices[node_id.index()] = glm::mat3(1.0f);

  node_to_transform[node_id.index()] = transforms.size() - 1;
}

void TransformManager::update_node_hierarchy(NodeID node_id, const glm::mat3& parent_global_mat) {
  TransformInfo* transform = get_transform(node_id.index());
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
  uint32_t child_idx = transform->first_child_idx;
  while (child_idx != INVALID_NODE_ID.index()) {
    update_node_hierarchy(NodeID::from(child_idx, 0), my_global_mat);
    TransformInfo* child_transform = get_transform(child_idx);
    child_idx = child_transform ? child_transform->next_sibling_idx : INVALID_NODE_ID.index();
  }
}

void TransformManager::on_node_destroyed(NodeID node_id) noexcept {
  if (!has_transform(node_id)) return;

  remove_from_parent(node_id);

  TransformIndex index = node_to_transform[node_id.index()];

  // Invalidate the memory (Ghost ID)
  transforms[index].node_idx = INVALID_NODE_ID.index();

  // Break the lookup link immediately
  node_to_transform[node_id.index()] = invalid_transform_index;
}

void TransformManager::on_frame_end() noexcept {
  size_t write_idx = 0;
  for (size_t read_idx = 0; read_idx < transforms.size(); ++read_idx) {
    if (transforms[read_idx].node_idx != INVALID_NODE_ID.index()) {
      if (write_idx != read_idx) {
        transforms[write_idx] = transforms[read_idx];
      }
      write_idx++;
    }
  }

  transforms.resize(write_idx);

  std::fill(node_to_transform.begin(), node_to_transform.end(), invalid_transform_index);
  for (size_t i = 0; i < transforms.size(); ++i) {
    node_to_transform[transforms[i].node_idx] = i;
  }
}

void TransformManager::remove_from_parent(NodeID child_id) noexcept {
  TransformInfo* child = get_transform(child_id.index());
  if (!child || child->parent_idx == INVALID_NODE_ID.index()) return;

  TransformInfo* parent = get_transform(child->parent_idx);
  if (!parent) return;

  // If this child is the first child, just point the parent to the next sibling
  if (parent->first_child_idx == child_id.index()) {
    parent->first_child_idx = child->next_sibling_idx;
  } else {
    // Otherwise, find the sibling immediately BEFORE this child, and patch the link
    uint32_t current_sibling_idx = parent->first_child_idx;
    while (current_sibling_idx != INVALID_NODE_ID.index()) {
      TransformInfo* sibling = get_transform(current_sibling_idx);
      if (!sibling) break;

      if (sibling->next_sibling_idx == child_id.index()) {
        sibling->next_sibling_idx = child->next_sibling_idx; // Bypass the removed child
        break;
      }
      current_sibling_idx = sibling->next_sibling_idx;
    }
  }

  // Clear the child's links
  child->parent_idx = INVALID_NODE_ID.index();
  child->next_sibling_idx = INVALID_NODE_ID.index();
}

void TransformManager::set_parent(NodeID child_id, NodeID new_parent_id) noexcept {
  TransformInfo* child = get_transform(child_id.index());
  if (!child) return;

  // Unlink from current parent to avoid corrupting the list
  if (child->parent_idx != INVALID_NODE_ID.index()) {
    remove_from_parent(child_id);
  }

  // Link to new parent
  if (new_parent_id != INVALID_NODE_ID) {
    TransformInfo* parent = get_transform(new_parent_id.index());
    if (parent) {
      child->parent_idx = new_parent_id.index();
      child->next_sibling_idx = parent->first_child_idx;
      parent->first_child_idx = child_id.index();
    }
  }

  mark_spatial_children_dirty(child_id);
}

void TransformManager::on_node_reparented(NodeID node_id, NodeID new_parent_id, NodeID old_parent_id) noexcept {
  set_parent(node_id, new_parent_id);
}

void TransformManager::on_scene_clear() noexcept {
  transforms.clear();
  std::fill(node_to_transform.begin(), node_to_transform.end(), invalid_transform_index);
}

Result<void> TransformManager::on_serialize_scene(nlohmann::json& json) const {
  json["nodes"] = nlohmann::json::array();

  for (const auto& transform : transforms) {
    // Fixed: Should be == to skip INVALID nodes. If it was != it would skip valid nodes!
    if (transform.node_idx == INVALID_NODE_ID.index()) {
      continue;
    }

    nlohmann::json transform_json;

    transform_json["id"] = transform.node_idx;

    transform_json["local_position"]["x"] = transform.local_position.x;
    transform_json["local_position"]["y"] = transform.local_position.y;

    transform_json["local_scale"]["x"] = transform.local_scale.x;
    transform_json["local_scale"]["y"] = transform.local_scale.y;

    transform_json["local_shear"]["x"] = transform.local_shear.x;
    transform_json["local_shear"]["y"] = transform.local_shear.y;

    transform_json["local_rotation"] = transform.local_rotation;

    transform_json["parent_id"] = transform.parent_idx;

    transform_json["first_child_id"] = transform.first_child_idx;

    transform_json["next_sibling_id"] = transform.next_sibling_idx;

    json["nodes"].push_back(transform_json);
  }

  return bird::ok();
}

Result<void> TransformManager::on_deserialize_scene(const nlohmann::json& json) {
  if (!json.contains("nodes") || !json["nodes"].is_array()) {
    return bird::ok();
  }

  for (const auto& node_json : json["nodes"]) {
    if (!node_json.contains("id") || !node_json["id"].is_number_integer()) {
      continue;
    }

    uint32_t node_idx = node_json["id"].get<uint32_t>();

    // Bounds check to ensure we don't exceed MAX_ACTIVE_NODES
    if (node_idx >= node_to_transform.size()) {
      continue;
    }

    // Try to get the transform. If the Scene hasn't allocated it yet, force allocation.
    TransformInfo* transform = get_transform(node_idx);
    if (!transform) {
      on_node_require_manager(NodeID::from(node_idx, 0));
      transform = get_transform(node_idx);

      // If it's STILL null, something is deeply wrong with memory, skip to be safe
      if (!transform) continue;
    }

    // Extract Position
    if (node_json.contains("local_position")) {
      transform->local_position.x = node_json["local_position"].value("x", 0.0f);
      transform->local_position.y = node_json["local_position"].value("y", 0.0f);
    }

    // Extract Scale
    if (node_json.contains("local_scale")) {
      transform->local_scale.x = node_json["local_scale"].value("x", 1.0f);
      transform->local_scale.y = node_json["local_scale"].value("y", 1.0f);
    }

    // Extract Shear
    if (node_json.contains("local_shear")) {
      transform->local_shear.x = node_json["local_shear"].value("x", 0.0f);
      transform->local_shear.y = node_json["local_shear"].value("y", 0.0f);
    }

    // Extract Rotation
    transform->local_rotation = node_json.value("local_rotation", 0.0f);

    // Extract Hierarchy Links (safely falling back to INVALID if missing)
    transform->parent_idx       = node_json.value("parent_id", INVALID_NODE_ID.index());
    transform->first_child_idx  = node_json.value("first_child_id", INVALID_NODE_ID.index());
    transform->next_sibling_idx = node_json.value("next_sibling_id", INVALID_NODE_ID.index());

    // CRITICAL: Force the engine to rebuild this node's global matrix on the next update
    transform->is_dirty = true;
  }

  return bird::ok();
}

} // bird::node_system::managers