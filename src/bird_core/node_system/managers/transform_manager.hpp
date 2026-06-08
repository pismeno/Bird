#pragma once

#include "node_system/inode_manager.hpp"

#include <vector>
#include <cstdint>
#include <limits>

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include "utils/result.hpp"
#include "node_system/node_types.hpp"

namespace bird::node_system::managers {

using TransformIndex = uint32_t;
inline constexpr TransformIndex INVALID_TRANSFORM_INDEX = std::numeric_limits<uint32_t>::max();

class TransformManager : public NodeManagerBase<TransformManager> {
 public:
  explicit TransformManager() {
    global_matrices.resize(MAX_ACTIVE_NODES, glm::mat3x3(1.0f));
    node_to_transform.resize(MAX_ACTIVE_NODES, INVALID_TRANSFORM_INDEX);
    transforms.reserve(MAX_ACTIVE_NODES);
  }

  static constexpr const char* MANAGER_NAME = "transform_manager";

  [[nodiscard]] bool has_transform(NodeID node_id) const noexcept;

  /**
   * Sets the parent of the given child node, in this manager, this is needed for mantaining the hierarchy.
   * @param child_id
   * @param parent_id
   */
  void set_parent(NodeID child_id, NodeID parent_id) noexcept;
  void remove_from_parent(NodeID child_id) noexcept;

  /**
   * @brief Gets the global transformation matrix of the given node, this represents scale, translation, shear and rotation.
   * @param node_id ID of the node to get the matrix for.
   * @return The 3x3 global transformation matrix.
   */
  [[nodiscard]] inline const glm::mat3x3& get_global_matrix(NodeID node_id) const noexcept { return global_matrices[node_id.index()]; }

  /**
   * @brief Gets all the global transformation matrices of all nodes in the scene.
   * @return reference to the vector of matrices, it is indexed by NodeID.index().
   */
  [[nodiscard]] inline const std::vector<glm::mat3x3>& get_all_matrices() const noexcept { return global_matrices; }

  Result<void> set_node_position(NodeID node_id, glm::vec2 pos);
  Result<void> set_node_scale(NodeID node_id, glm::vec2 scale);
  Result<void> set_node_shear(NodeID node_id, glm::vec2 shear);
  Result<void> set_node_rotation(NodeID node_id, float rotation);

  void on_update() noexcept override;
  void on_node_destroyed(NodeID node_id) noexcept override;
  void on_frame_end() noexcept override;
  void on_node_require_manager(NodeID node_id) noexcept override;
  void on_node_reparented(NodeID node_id, NodeID new_parent_id, NodeID old_parent_id) noexcept override;
  void on_scene_clear() noexcept override;
  Result<void> on_serialize_scene(nlohmann::json& json) const override;
  Result<void> on_deserialize_scene(const nlohmann::json& json) override;

 private:
  /**
   * @brief private information for nodes
   */
  struct TransformInfo {
    glm::vec2 local_position = {0.0f, 0.0f};
    glm::vec2 local_scale = {1.0f, 1.0f};
    glm::vec2 local_shear = {0.0f, 0.0f};
    float local_rotation = 0.0f;

    uint32_t node_idx = INVALID_NODE_ID.index();
    uint32_t parent_idx = INVALID_NODE_ID.index();
    uint32_t first_child_idx = INVALID_NODE_ID.index();
    uint32_t next_sibling_idx = INVALID_NODE_ID.index();

    bool is_dirty = true;
  };

  TransformInfo* get_transform(uint32_t node_idx);
  void update_node_hierarchy(NodeID node_id, const glm::mat3& parent_global_mat);
  void mark_spatial_children_dirty(NodeID parent_id);

  std::vector<TransformInfo> transforms;           // Indexed by NodeID
  std::vector<glm::mat3x3> global_matrices;      // Sorted hierarchically by tree depth
  std::vector<TransformIndex> node_to_transform; // The Lookup Table: index is NodeID, value is TransformIndex
};

} // bird::node_system::managers