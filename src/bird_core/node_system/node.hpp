#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "node_types.hpp"

namespace bird::node_system {

class Scene;

class Node {

  friend class Scene;

 public:
  Node(std::string name);

  inline NodeID getParentId() const { return parentId; }
  inline NodeID getId() const { return id; }
  inline std::string_view getName() const { return name; }
  void addChild(NodeID child_id, Scene& scene);

  virtual void on_enter_scene(Scene& scene) {}

 private:
  NodeID id = INVALID_NODE_ID;
  NodeID parentId = INVALID_NODE_ID;
  std::vector<NodeID> childrenIds;

  std::string name;
};

} // bird::node_system