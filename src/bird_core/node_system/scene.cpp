#include "scene.hpp"

#include <memory>
#include <vector>

#include "managers/transform/transform_manager.hpp"

namespace bird::node_system {

using namespace managers;

std::unique_ptr<Scene> Scene::create() {
  auto scene = std::make_unique<Scene>();
  scene->add_manager<managers::TransformManager>();
  return scene;
}

[[nodiscard]] NodeID Scene::createNode(NodeID parent_id, std::string name){
  NodeID newId = idCounter.fetch_add(1, std::memory_order_relaxed);

  auto node = std::make_unique<Node>(std::move(name));
  node->id = newId;
  node->parentId = INVALID_NODE_ID;

  nodes[newId] = std::move(node);

  if (parent_id != INVALID_NODE_ID && parent_id != newId) {
    if (Node* parent = getNode(parent_id)) {
      parent->addChild(newId, *this);
    }
  }

  for (auto& manager : managers) {
    manager->on_node_created(newId);
  }

  return newId;
}

[[nodiscard]] NodeID Scene::createNode2D(NodeID parent_id, std::string name) {
  NodeID node_id = createNode(parent_id, name);
  auto transform_manager = get_manager<TransformManager>();
  transform_manager->create_transform(node_id);
  transform_manager->set_parent(node_id, parent_id);
  return node_id;
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