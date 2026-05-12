#include "node_manager.hpp"

namespace bird::nodes {

[[nodiscard]] Node* NodeManager::getNode(NodeID id) {
  {
    auto it = nodes.find(id);
    return (it != nodes.end()) ? it->second.get() : nullptr;
  }
}

} // bird::nodes