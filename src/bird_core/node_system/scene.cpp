#include "scene.hpp"
#include "node_types.hpp"

#include <memory>
#include <utility>
#include <vector>
#include <cstdint>
#include <algorithm>

#include "managers/transform_manager.hpp"
#include "managers/drawable_manager.hpp"

namespace bird::node_system {

using namespace managers;

std::unique_ptr<Scene> Scene::create_scene_2d() {
  auto scene = std::make_unique<Scene>();
  scene->add_manager<TransformManager>();
  scene->add_manager<DrawableManager>();
  return scene;
}

[[nodiscard]] NodeID Scene::generate_id() {
  if (!recycled_ids.empty()) {
    uint32_t recycled = recycled_ids.back();
    recycled_ids.pop_back();
    uint32_t current_gen = generations[recycled];

    return NodeID::from(recycled, current_gen);
  }

  if (next_unused_id >= MAX_ACTIVE_NODES) {
    return INVALID_NODE_ID;
  }

  uint32_t new_id = next_unused_id++;
  generations[new_id] = 0;

  return NodeID::from(new_id, 0);
}

[[nodiscard]] NodeID Scene::createNode(NodeID parent_id, std::string name){
  NodeID new_id = generate_id();

  nodes[new_id.index()].id = new_id;
  nodes[new_id.index()].parentId = INVALID_NODE_ID;
  nodes[new_id.index()].name = std::move(name);

  if (parent_id != INVALID_NODE_ID && parent_id != new_id) {
    reparent_node(new_id, parent_id);
  }

  for (auto& manager : managers) {
    manager->on_node_created(new_id);
  }

  return new_id;
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
  uint32_t idx = node_id.index();

  if (idx >= nodes.size()) {
    return nullptr;
  }

  if (node_id.generation() != generations[idx]) {
    return nullptr;
  }

  return &nodes[idx];
}

Result Scene::destroy_node(NodeID node_id) {
  Node* node = getNode(node_id);
  if (!node) {
    return Result::fail("Node with provided ID not found");
  }

  if (node->parentId != INVALID_NODE_ID) {
    Node* parent = getNode(node->parentId);
    if (parent) {
      auto& siblings = parent->childrenIds;
      siblings.erase(std::remove(siblings.begin(), siblings.end(), node_id), siblings.end());
    }
  }

  std::vector<NodeID> children_to_kill = node->childrenIds;
  for (NodeID child_id : children_to_kill) {
    destroy_node(child_id);
  }

  for (auto& manager : managers) {
    manager->on_node_destroyed(node_id);
  }

  uint32_t idx = node_id.index();

  generations[idx]++;

  recycled_ids.push_back(idx);

  node->id = INVALID_NODE_ID;
  node->parentId = INVALID_NODE_ID;
  node->childrenIds.clear();
  node->name.clear();

  return Result::ok();
}

Result Scene::reparent_node(NodeID node_id, NodeID new_parent_id) {
  if (node_id == new_parent_id) {
    return Result::fail("Cannot parent a node to itself.");
  }

  Node *child = getNode(node_id);
  Node *new_parent = getNode(new_parent_id);

  if (!child) return Result::fail("Node with provided ID not found");
  if (!new_parent) return Result::fail("Parent node with provided ID not found");

  if (child->parentId != INVALID_NODE_ID) {
    Node *old_parent = getNode(child->parentId);
    if (old_parent) {
      auto& siblings = old_parent->childrenIds;
      siblings.erase(std::remove(siblings.begin(), siblings.end(), node_id), siblings.end());
    }
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