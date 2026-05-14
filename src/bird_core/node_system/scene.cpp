#include "scene.hpp"

#include <memory>
#include <utility>
#include <vector>

#include "managers/transform_manager.hpp"
#include "managers/drawable_manager.hpp"

namespace bird::node_system {

using namespace managers;

std::unique_ptr<Scene> Scene::create() {
  auto scene = std::make_unique<Scene>();
  scene->add_manager<TransformManager>();
  scene->add_manager<DrawableManager>();
  return scene;
}

[[nodiscard]] NodeID Scene::createNode(NodeID parent_id, std::string name){
  NodeID newId = idCounter.fetch_add(1, std::memory_order_relaxed);

  auto node = std::make_unique<Node>(std::move(name));
  node->id = newId;
  node->parentId = INVALID_NODE_ID;

  nodes[newId] = std::move(node);

  if (parent_id != INVALID_NODE_ID && parent_id != newId) {
    reparent_node(newId, parent_id);
  }

  for (auto& manager : managers) {
    manager->on_node_created(newId);
  }

  return newId;
}

[[nodiscard]] NodeID Scene::createNode2D(NodeID parent_id, std::string name) {
  NodeID node_id = createNode(parent_id, std::move(name));
  auto transform_manager = get_manager<TransformManager>();
  transform_manager->create_transform(node_id);
  transform_manager->set_parent(node_id, parent_id);
  return node_id;
}

[[nodiscard]] NodeID Scene::createNode2DDrawable(NodeID parent_id, std::string name) {
  NodeID node_id = createNode2D(parent_id, std::move(name));
  auto drawable_manager = get_manager<DrawableManager>();
  drawable_manager->create_drawable(node_id);
  return node_id;
}

[[nodiscard]] Node* Scene::getNode(NodeID node_id) {
  {
    auto it = nodes.find(node_id);
    return (it != nodes.end()) ? it->second.get() : nullptr;
  }
}

Result Scene::destroy_node(NodeID node_id) {
  Node* node = getNode(node_id);
  if (!node) {
    return Result::fail("Node with provided ID not found");
  }

  std::vector<NodeID> children_to_kill = node->childrenIds;
  for (NodeID child_id : children_to_kill) {
    destroy_node(child_id);
  }

  for (auto& manager : managers) {
    manager->on_node_destroyed(node_id);
  }

  nodes.erase(node_id);
  return Result::ok();
}

Result Scene::reparent_node(NodeID node_id, NodeID new_parent_id) {
  Node *child = getNode(node_id);
  Node *new_parent = getNode(new_parent_id);

  if (!child) {
    return Result::fail("Node with provided ID not found");
  }

  if (!new_parent) {
    return Result::fail("Parent node with provided ID not found");
  }

  child->parentId = new_parent_id;
  new_parent->childrenIds.push_back(node_id);

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