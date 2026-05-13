#include "node_types.hpp"
#include "node.hpp"
#include "scene.hpp"

#include <string>
#include <utility>

namespace bird::node_system {

Node::Node(std::string name) : name(std::move(name)) {}

void Node::addChild(NodeID child_id, Scene &scene) {
  Node *child = scene.getNode(child_id);
  if (child) {
    child->parentId = this->id;
    childrenIds.push_back(child_id);
  }
}

} // bird::node_system