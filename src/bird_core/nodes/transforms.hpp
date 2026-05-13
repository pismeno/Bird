#pragma once

#include <glm/ext/matrix_float3x3.hpp>
#include <glm/vec2.hpp>

#include "node_types.hpp"

namespace bird::nodes {

struct Transform2D {
  glm::vec2 local_position = {0.0f, 0.0f};
  glm::vec2 local_scale = {1.0f, 1.0f};
  float local_rotation = 0.0f;

  glm::mat3x3 global_matrix = glm::mat3x3(1.0f);

  NodeID node_id = INVALID_NODE_ID;
  int spatial_parent_index = -1;
  bool is_dirty = true;
};

} // bird::nodes