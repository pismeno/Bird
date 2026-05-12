#include "scene.hpp"

namespace bird::nodes {

[[nodiscard]] Node* Scene::getNode(NodeID id) {
  {
    auto it = nodes.find(id);
    return (it != nodes.end()) ? it->second.get() : nullptr;
  }
}

} // bird::nodes