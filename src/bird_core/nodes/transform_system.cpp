#include "transform_system.hpp"
#include "transforms.hpp"

#include <glm/glm.hpp>

namespace bird::nodes {

void TransformSystem::ensure_capacity(size_t capacity) {
  if (node_to_transform.size() < capacity) {
    node_to_transform.resize(capacity, -1);
  }
}

void TransformSystem::add_transform(NodeID node_id) {
  ensure_capacity(transforms.size() + 1);

  transforms.push_back(Transform2D());
  transforms.back().node_id = node_id;

  node_to_transform[node_id] = transforms.size() - 1;
}

bool TransformSystem::has_transform(NodeID node_id) const {
  if (node_id >= node_to_transform.size()) return false;
  return node_to_transform[node_id] != -1;
}

Transform2D* TransformSystem::get_transform(NodeID node_id) {
  if (node_id >= node_to_transform.size()) {
    return nullptr;
  }

  int index = node_to_transform[node_id];
  if (index == -1) {
    return nullptr;
  }

  return &transforms[index];
}

void TransformSystem::update_dirty_transforms() {
  for (size_t i = 0; i < transforms.size(); i++) {
    Transform2D& transform = transforms[i];

    if (transform.is_dirty) {

      // 1. Fast Manual 3x3 Matrix Construction (TRS)
      float cos = std::cos(transform.local_rotation);
      float sin = std::sin(transform.local_rotation);

      // GLM is Column-Major: mat3(col0, col1, col2)
      glm::mat3 local_mat(
          cos * transform.local_scale.x,  sin * transform.local_scale.x, 0.0f,  // Column 0: X-axis
          -sin * transform.local_scale.y,  cos * transform.local_scale.y, 0.0f,  // Column 1: Y-axis
          transform.local_position.x,   transform.local_position.y,  1.0f   // Column 2: Translation
      );

      // 2. Hierarchical Composition
      if (transform.spatial_parent_index != -1) {
        // IMPORTANT: Parent is guaranteed to be updated already
        // because the array is sorted Depth-First (0 to N).
        transform.global_matrix = transforms[transform.spatial_parent_index].global_matrix * local_mat;
      } else {
        // Root node: Global is just Local
        transform.global_matrix = local_mat;
      }

      // 3. Reset flag
      transform.is_dirty = false;
    }
  }
}


} // bird::nodes