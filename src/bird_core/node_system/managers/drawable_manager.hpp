#pragma once

#include "node_system/imanager.hpp"

#include <vector>
#include <cstdint>
#include <limits>

#include "node_system/node_types.hpp"

namespace bird::node_system::managers {

struct Drawable {
  bool is_visible = false;

  //uint32_t texture_id = 0; // in future?
  uint32_t z_index = 0;
};

class DrawableManager : public IManager {
 public:
  DrawableManager() {
    drawables.resize(MAX_ACTIVE_NODES, Drawable{});
  }

  inline void create_drawable(NodeID node_id) { drawables[node_id.index()].is_visible = true; }
  [[nodiscard]] inline bool has_drawable(NodeID node_id) const { return drawables[node_id.index()].is_visible; }
  [[nodiscard]] inline const Drawable& get_drawable(NodeID node_id) const { return drawables[node_id.index()]; }
  [[nodiscard]] inline const std::vector<Drawable>& get_all_drawables() const { return drawables; }

  void on_node_destroyed(NodeID node_id) override;

 private:
  std::vector<Drawable> drawables;
};

} // bird::node_system::managers