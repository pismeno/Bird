#pragma once

#include "transforms.hpp"
#include "node_system/node_types.hpp"
#include "node_system/imanager.hpp"

#include <vector>

#include <glm/glm.hpp>

#include "utils/result.hpp"

namespace bird::node_system::managers {

class TransformManager : public IManager {
 public:
  void ensure_capacity(size_t capacity);
  void create_transform(NodeID node_id);
  bool has_transform(NodeID node_id) const;
  glm::mat3x3 get_global_matrix(NodeID node_id);

  void set_parent(NodeID child_id, NodeID parent_id);
  void remove_from_parent(NodeID child_id);

  Result set_node_position(NodeID node_id, glm::vec2 pos);
  void mark_spatial_children_dirty(NodeID parent_id);

  void on_update() override;
  void on_node_destroyed(NodeID node_id) override;
  void on_frame_end() override;

 private:
  Transform2D* get_transform(NodeID node_id);

  // The fast math array (Data)
  std::vector<Transform2D> transforms;
  std::vector<glm::mat3x3> global_matrices;

  // The Lookup Table: index is NodeID, value is TransformIndex
  // Initialize with INVALID_TRANSFORM_INDEX (meaning no transform)
  std::vector<TransformIndex> node_to_transform;
};

} // bird::node_system::managers