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

  inline const NodeID getId() const noexcept { return id; }
  inline const NodeID getParentId() const noexcept { return parentId; }
  inline const std::vector<NodeID>& getChildrenIds() const noexcept { return childrenIds; }

 private:
  NodeID id = INVALID_NODE_ID;
  NodeID parentId = INVALID_NODE_ID;
  std::vector<NodeID> childrenIds;
};

} // bird::node_system