#pragma once

#include "node_system/imanager.hpp"

#include <vector>
#include <cstdint>
#include <limits>

#include <glm/glm.hpp>

#include "utils/result.hpp"

namespace bird::node_system::managers {

using TransformIndex = uint32_t;
inline constexpr TransformIndex INVALID_TRANSFORM_INDEX = std::numeric_limits<uint32_t>::max();

class TransformManager : public IManager {
 public:
  void create_transform(NodeID node_id);
  [[nodiscard]] bool has_transform(NodeID node_id) const;
  void set_parent(NodeID child_id, NodeID parent_id);
  void remove_from_parent(NodeID child_id);

  glm::mat3x3 get_global_matrix(NodeID node_id);

  Result set_node_position(NodeID node_id, glm::vec2 pos);
  void mark_spatial_children_dirty(NodeID parent_id);

  void on_update() override;
  void on_node_destroyed(NodeID node_id) override;
  void on_frame_end() override;

 private:
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

  Transform2D* get_transform(NodeID node_id);
  void ensure_capacity(size_t capacity);

  // The fast math array (Data)
  std::vector<Transform2D> transforms;
  std::vector<glm::mat3x3> global_matrices;

  // The Lookup Table: index is NodeID, value is TransformIndex
  // Initialize with INVALID_TRANSFORM_INDEX (meaning no transform)
  std::vector<TransformIndex> node_to_transform;
};

} // bird::node_system::managers