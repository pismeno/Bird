#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "node_types.hpp"

namespace bird::nodes {

class NodeManager;

class Node {

  friend class NodeManager;

 public:
  Node(std::string name);

  inline Node* getParent(NodeManager& manager);
  inline NodeID getId() const { return id; }
  inline std::string_view getName() const { return name; }
  void addChild(NodeID child_id, NodeManager& manager);

 private:
  NodeID id = INVALID_NODE_ID;
  NodeID parentId = INVALID_NODE_ID;
  std::vector<NodeID> childrenIds;

  std::string name;
};

} // bird::nodes