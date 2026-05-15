#pragma once

#include <cstdint>
#include <limits>

namespace bird::node_system {

struct NodeID {
  uint64_t id;

  static inline NodeID from(uint32_t index, uint32_t generation) {
    uint64_t packed = (static_cast<uint64_t>(generation) << 32) | index;
    return NodeID{ packed };
  }

  inline uint32_t index() const {
    return static_cast<uint32_t>(id);
  }

  inline uint32_t generation() const {
    return static_cast<uint32_t>(id >> 32);
  }

  inline bool operator==(const NodeID& other) const {
    return id == other.id;
  }
  inline bool operator!=(const NodeID& other) const {
    return id != other.id;
  }
};

inline constexpr uint32_t MAX_ACTIVE_NODES = 50000;
inline constexpr NodeID INVALID_NODE_ID = { std::numeric_limits<uint32_t>::max() };

} // bird::node_system