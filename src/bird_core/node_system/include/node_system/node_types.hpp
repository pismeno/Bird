#pragma once

#include <cstdint>
#include <limits>
#include <vector>
#include <string>

namespace bird {

/**
 * @brief Unique identifier for a node. It consists of a 32-bit index and a 32-bit generation-counter.
 */
struct NodeID {
  uint64_t id;

  static inline NodeID from(uint32_t index, uint32_t generation) noexcept {
    uint64_t packed = (static_cast<uint64_t>(generation) << 32) | index;
    return NodeID{ packed };
  }

  /**
   * @return the index part of the NodeID.
   */
  [[nodiscard]] inline uint32_t index() const noexcept {
    return static_cast<uint32_t>(id);
  }

  /**
   * @return the generation-counter part of the NodeID.
   */
  [[nodiscard]] inline uint32_t generation() const noexcept {
    return static_cast<uint32_t>(id >> 32);
  }

  inline bool operator==(const NodeID& other) const noexcept {
    return id == other.id;
  }
  inline bool operator!=(const NodeID& other) const noexcept {
    return id != other.id;
  }
};

/**
 * @brief Maximum number of active nodes, hard-coded for now.
 */
inline constexpr uint32_t MAX_ACTIVE_NODES = 50000;

/**
 * @brief Invalid NodeID. Explicitly sets both index and generation to max to avoid collisions.
 */
inline constexpr NodeID INVALID_NODE_ID = { std::numeric_limits<uint64_t>::max() };

} // bird