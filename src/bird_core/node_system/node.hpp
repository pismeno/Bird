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
  Node(std::string name, std::string type) : name(std::move(name)), type(std::move(type)) {};

  inline const NodeID getId() const { return id; }
  inline const NodeID getParentId() const { return parentId; }
  inline const std::vector<NodeID>& getChildrenIds() const { return childrenIds; }
  inline const std::string_view getName() const { return name; }
  inline const std::string_view getType() const { return type; }

 private:
  NodeID id = INVALID_NODE_ID;
  NodeID parentId = INVALID_NODE_ID;
  std::vector<NodeID> childrenIds;

  std::string name;
  std::string type;
};

} // bird::node_system