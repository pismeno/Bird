#pragma once

#include "node_system/imanager.hpp"

#include <vector>
#include <cstdint>
#include <limits>

#include <glm/glm.hpp>

#include "utils/result.hpp"
#include "node_system/node_types.hpp"

namespace bird::node_system::managers {

using TransformIndex = uint32_t;
inline constexpr TransformIndex INVALID_TRANSFORM_INDEX = std::numeric_limits<uint32_t>::max();

class TransformManager : public IManager {
 public:
  TransformManager() {
    global_matrices.resize(MAX_ACTIVE_NODES, glm::mat3x3(1.0f));
    node_to_transform.resize(MAX_ACTIVE_NODES, INVALID_TRANSFORM_INDEX);
  }
  void create_transform(NodeID node_id);
  [[nodiscard]] bool has_transform(NodeID node_id) const;
  void set_parent(NodeID child_id, NodeID parent_id);
  void remove_from_parent(NodeID child_id);

  inline const glm::mat3x3& get_global_matrix(NodeID node_id) const { return global_matrices[node_id.index()]; }

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
  void update_node_hierarchy(NodeID node_id, const glm::mat3& parent_global_mat);

  std::vector<Transform2D> transforms;           // Indexed by NodeID
  std::vector<glm::mat3x3> global_matrices;      // Sorted hierarchically by tree depth
  std::vector<TransformIndex> node_to_transform; // The Lookup Table: index is NodeID, value is TransformIndex
};

} // bird::node_system::managers