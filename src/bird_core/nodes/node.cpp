#include "node_types.hpp"
#include "node.hpp"
#include "node_manager.hpp"

namespace bird::nodes {

Node* Node::getParent(NodeManager& manager) {
  if (parentId == INVALID_NODE_ID) return nullptr;
  return manager.getNode(parentId);
}

void Node::addChild(NodeID child_id, NodeManager &manager) {
  Node *child = manager.getNode(child_id);
  if (child) {
    child->parentId = this->id;
    childrenIds.push_back(child_id);
  }
}

} // bird::nodes