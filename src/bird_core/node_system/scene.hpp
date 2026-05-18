#pragma once

#include "node_types.hpp"
#include "node.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <stdexcept>

#include <glm/vec2.hpp>

#include "utils/result.hpp"
#include "node_system/inode_manager.hpp"
#include "node_system/node_handle.hpp"

namespace bird::node_system {

class Scene {

  friend class NodeHandle;
  friend class SceneFactory;

 public:
  Scene() {
    nodes.resize(MAX_ACTIVE_NODES);
    generations.resize(MAX_ACTIVE_NODES);
  }

  [[nodiscard]] NodeID create_node(NodeID parent_id, const std::string& node_type, std::string name);

  [[nodiscard]] NodeHandle get_node(NodeID node_id);
  Result destroy_node(NodeID node_id);
  Result reparent_node(NodeID node_id, NodeID new_parent_id);

  /**
   * @return The highest index part of NodeID of a node that exists.
   */
  [[nodiscard]] inline uint32_t get_highest_allocated_node_index() const { return next_unused_id.load(); }

  void update();
  void end_frame();

  template <typename T>
  T* get_manager() const {
    auto it = managers.find(T::MANAGER_NAME);
    if (it != managers.end()) {
      return static_cast<T*>(it->second.get());
    }
    return nullptr;
  }

 private:
  [[nodiscard]] NodeID generate_id();

  std::atomic<uint32_t> next_unused_id {0};
  std::vector<uint32_t> recycled_ids;
  std::vector<uint32_t> generations; // Index = the raw array slot, Value = the current active generation
  std::vector<Node> nodes;

  std::unordered_map<std::string, std::unique_ptr<INodeManager>> managers;

  std::unordered_map<std::string, NodeDefinition> node_types;
};

} // bird::node_system