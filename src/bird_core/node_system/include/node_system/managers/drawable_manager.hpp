#pragma once

#include <node_system/inode_manager.hpp>

#include <vector>
#include <cstdint>
#include <limits>

#include <node_system/node_types.hpp>

namespace bird::node_system::managers {

struct Drawable {
  bool is_visible = false;

  //uint32_t texture_id = 0; // in future?
  uint32_t z_index = 0;
};

class DrawableManager : public NodeManagerBase<DrawableManager> {
 public:
  explicit DrawableManager() {
    drawables.resize(MAX_ACTIVE_NODES, Drawable{});
  }

  static constexpr const char* MANAGER_NAME = "drawable_manager";

  [[nodiscard]] inline bool has_drawable(NodeID node_id) const noexcept { return drawables[node_id.index()].is_visible; }

  /**
   * Gets the draw information of the given node.
   * @param node_id ID of the node to get the draw information for.
   * @return Drawable struct tied to this node
   */
  [[nodiscard]] inline const Drawable& get_drawable(NodeID node_id) const noexcept { return drawables[node_id.index()]; }

  /**
   * Gets all the draw information of all nodes in the scene.
   * @return reference to the vector of drawables, it is indexed by NodeID.index().
   */
  [[nodiscard]] inline const std::vector<Drawable>& get_all_drawables() const noexcept { return drawables; }

  void on_node_destroyed(NodeID node_id) noexcept override;
  void on_node_require_manager(NodeID node_id) noexcept override;
  void on_scene_clear() noexcept override;
  Result<void> on_serialize_scene(nlohmann::json& json) const override;
  Result<void> on_deserialize_scene(const nlohmann::json& json) override;

 private:
  std::vector<Drawable> drawables;
};

} // bird::node_system::managers