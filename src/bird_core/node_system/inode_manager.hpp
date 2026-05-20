#pragma once

#include <concepts>
#include <cstddef>
#include <string_view>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

#include <nlohmann/json.hpp>

#include "node_types.hpp"
#include "utils/result.hpp"

namespace bird::node_system {

using namespace bird;

class INodeManager {
 public:
  virtual ~INodeManager() = default;

  [[nodiscard]] virtual std::string get_name() const = 0;

  virtual void on_node_destroyed(NodeID node_id) {};
  virtual void on_node_created(NodeID node_id) {};
  virtual void on_node_require_manager(NodeID node_id) {};
  virtual void on_node_reparented(NodeID node_id, NodeID new_parent_id, NodeID old_parent_id) {};
  virtual void on_update() {};
  virtual void on_frame_end() {};
  virtual void on_scene_clear() {};
  virtual Result on_serialize_scene(nlohmann::json& json) const { return Result::ok(); };
  virtual Result on_deserialize_scene(const nlohmann::json& json) { return Result::ok(); };

  template <std::derived_from<INodeManager> T>
  static std::size_t get_type_id() {
    static std::size_t id = next_id++;
    return id;
  }

 private:
  static std::size_t next_id;
};

template <typename Derived>
class NodeManagerBase : public INodeManager {
 public:
  std::string get_name() const override {
    return std::string(Derived::MANAGER_NAME);
  }
};

} // bird::node_system
