#pragma once

#include "node_system/imanager.hpp"

#include <vector>
#include <cstdint>
#include <limits>

#include "node_system/node_types.hpp"

namespace bird::node_system::managers {

using DrawableIndex = uint32_t;
inline constexpr DrawableIndex INVALID_DRAWABLE_INDEX = std::numeric_limits<uint32_t>::max();

struct Drawable {
  NodeID node_id = INVALID_NODE_ID;

  //uint32_t texture_id = 0; // in future?
  uint32_t z_index = 0;
};

class DrawableManager : public IManager {
 public:
  void create_drawable(NodeID node_id);
  [[nodiscard]] bool has_drawable(NodeID node_id) const;
  Drawable* get_drawable(NodeID node_id);
  inline std::vector<Drawable>& get_drawables() { return drawables; }

  void on_node_destroyed(NodeID node_id) override;

 private:
  void ensure_capacity(size_t capacity);

  std::vector<Drawable> drawables;

  std::vector<DrawableIndex> node_to_drawable;
};

} // bird::node_system::managers