#pragma once

#include "node_types.hpp"
#include "node.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <unordered_map>

namespace bird::nodes {

class NodeManager {
 public:
  template<typename T, typename... Args>
  [[nodiscard]] NodeID createNode(NodeID parent_id, Args&&... args) {
    NodeID newId = idCounter.fetch_add(1, std::memory_order_relaxed);

    auto node = std::make_unique<T>(std::forward<Args>(args)...);
    node->id = newId;
    node->parentId = parent_id;

    Node* nodePtr = node.get();
    nodes[newId] = std::move(node);

    if (parent_id != INVALID_NODE_ID) {
      if (Node* parent = getNode(parent_id)) {
        parent->addChild(newId, *this);
      }
    }

    //nodePtr->_ready();

    return newId;
  }

  [[nodiscard]] Node* getNode(NodeID id);

 private:
  std::atomic<NodeID> idCounter{0};
  std::unordered_map<NodeID, std::unique_ptr<Node>> nodes;
};

} // bird::nodes
