#include "scene.hpp"

#include <memory>
#include <vector>

#include "managers/transform/transform_manager.hpp"

namespace bird::node_system {

std::unique_ptr<Scene> Scene::create() {
  auto scene = std::make_unique<Scene>();
  scene->add_manager<managers::TransformManager>();
  return scene;
}

[[nodiscard]] Node* Scene::getNode(NodeID id) {
  {
    auto it = nodes.find(id);
    return (it != nodes.end()) ? it->second.get() : nullptr;
  }
}

Result Scene::destroy_node(NodeID id) {
  Node* node = getNode(id);
  if (!node) {
    return Result::fail("Node with provided ID not found");
  }

  std::vector<NodeID> children_to_kill = node->childrenIds;
  for (NodeID child_id : children_to_kill) {
    destroy_node(child_id);
  }

  for (auto& manager : managers) {
    manager->on_node_destroyed(id);
  }

  nodes.erase(id);
  return Result::ok();
}

void Scene::end_frame() {
  for (auto& manager : managers) {
    manager->on_frame_end();
  }
}

void Scene::update() {
  for (auto& manager : managers) {
    manager->on_update();
  }
}

} // bird::node_system