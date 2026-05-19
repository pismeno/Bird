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
  Node() = default;

  inline const NodeID getId() const { return id; }
  inline const NodeID getParentId() const { return parentId; }
  inline const std::vector<NodeID>& getChildrenIds() const { return childrenIds; }

 private:
  NodeID id = INVALID_NODE_ID;
  NodeID parentId = INVALID_NODE_ID;
  std::vector<NodeID> childrenIds;
};

} // bird::node_system