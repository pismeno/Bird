#pragma once

#include <cstdint>
#include <vector>

#include "node_types.hpp"

namespace bird::nodes {

class NodeManager;

class Node {

  friend class NodeManager;

 public:
  inline Node* getParent(NodeManager& manager);
  inline NodeID getId() const { return id; }
  void addChild(NodeID child_id, NodeManager& manager);

 private:
  NodeID id;
  NodeID parentId;
  std::vector<NodeID> childrenIds;
};

} // bird::nodes