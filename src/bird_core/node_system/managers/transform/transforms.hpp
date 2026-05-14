#pragma once

#include <glm/ext/matrix_float3x3.hpp>
#include <glm/vec2.hpp>

#include "../../node_types.hpp"

namespace bird::node_system::managers {

struct Transform2D {
  glm::vec2 local_position = {0.0f, 0.0f};
  glm::vec2 local_scale = {1.0f, 1.0f};
  float local_rotation = 0.0f;

  NodeID node_id = INVALID_NODE_ID;
  NodeID parent_id = INVALID_NODE_ID;
  NodeID first_child_id = INVALID_NODE_ID;
  NodeID next_sibling_id = INVALID_NODE_ID;

  bool is_dirty = true;
};

} // bird::node_system::managers
