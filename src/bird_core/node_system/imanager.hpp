#pragma once

#include <cstddef>
#include "node_types.hpp"

namespace bird::node_system {

class IManager {
 public:
  virtual ~IManager() = default;

  virtual void on_node_destroyed(NodeID node_id) {};
  virtual void on_node_created(NodeID node_id) {};
  virtual void on_update() {};
  virtual void on_frame_end() {};

  template <typename T>
  static std::size_t get_type_id() {
    static std::size_t id = next_id++;
    return id;
  }

 private:
  static std::size_t next_id;
};

} // bird::node_system
