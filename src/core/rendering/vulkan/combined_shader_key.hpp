#pragma once

#include <cstdint>

namespace mole {

struct CombinedShaderKey {
  uint64_t vert_id;
  uint64_t frag_id;

  bool operator==(const CombinedShaderKey& other) const {
    return vert_id == other.vert_id && frag_id == other.frag_id;
  }

  bool operator<(const CombinedShaderKey& other) const {
    if (vert_id != other.vert_id) return vert_id < other.vert_id;
    return frag_id < other.frag_id;
  }
};

struct CombinedShaderKeyHash {
  std::size_t operator()(const CombinedShaderKey& k) const {
    return std::hash<uint64_t>()(k.vert_id) ^ (std::hash<uint64_t>()(k.frag_id) << 1);
  }
};

} // mole