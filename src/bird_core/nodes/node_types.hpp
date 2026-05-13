#pragma once

#include <cstdint>
#include <limits>

namespace bird::nodes {

using NodeID = uint64_t;
using TransformIndex = uint32_t;

inline constexpr NodeID INVALID_NODE_ID = std::numeric_limits<uint32_t>::max();
inline constexpr TransformIndex INVALID_TRANSFORM_INDEX = std::numeric_limits<uint32_t>::max();

} // bird::nodes