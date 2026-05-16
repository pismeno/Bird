#include "node_handle.hpp"

#include "node_system/scene.hpp"

namespace bird::node_system {

[[nodiscard]] bool NodeHandle::is_valid() const {
  return scene->generations[node_id.index()] == node_id.generation();
}

[[nodiscard]] Node* NodeHandle::get() const {
  if (!is_valid()) return nullptr;
  return &scene->nodes[node_id.index()];
}

} // bird::node_system