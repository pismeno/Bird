#include "drawable_manager.hpp"

namespace bird::node_system::managers {

void DrawableManager::on_node_destroyed(NodeID node_id) {
  drawables[node_id.index()].is_visible = false;
}

void DrawableManager::on_node_require_manager(NodeID node_id) {
  drawables[node_id.index()].is_visible = true;
}

} // bird::node_system::managers