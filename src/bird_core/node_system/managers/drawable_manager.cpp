#include "drawable_manager.hpp"

namespace bird::node_system::managers {

void DrawableManager::ensure_capacity(size_t capacity) {
  if (node_to_drawable.size() < capacity) {
    node_to_drawable.resize(capacity, INVALID_DRAWABLE_INDEX);
  }
}

Drawable* DrawableManager::get_drawable(NodeID node_id) {
  if (node_id.index() >= node_to_drawable.size()) {
    return nullptr;
  }

  DrawableIndex index = node_to_drawable[node_id.index()];
  if (index == INVALID_DRAWABLE_INDEX || static_cast<size_t>(index) >= drawables.size()) {
    return nullptr;
  }

  return &drawables[static_cast<size_t>(index)];
}

bool DrawableManager::has_drawable(NodeID node_id) const {
  if (node_id.index() >= node_to_drawable.size()) return false;
  return node_to_drawable[node_id.index()] != INVALID_DRAWABLE_INDEX;
}

void DrawableManager::on_node_destroyed(NodeID node_id) {
  if (!has_drawable(node_id)) return;

  DrawableIndex index = node_to_drawable[node_id.index()];

  // 1. Invalidate the memory (Ghost ID)
  drawables[static_cast<size_t>(index)].node_id = INVALID_NODE_ID;

  // 2. Break the lookup link immediately
  node_to_drawable[node_id.index()] = INVALID_DRAWABLE_INDEX;
}

void DrawableManager::create_drawable(NodeID node_id) {
  ensure_capacity(node_id.index() + 1);

  drawables.push_back(Drawable());
  drawables.back().node_id = node_id;

  node_to_drawable[node_id.index()] = static_cast<DrawableIndex>(drawables.size() - 1);
}

} // bird::node_system::managers