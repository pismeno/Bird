#pragma once

#include <cstdint>

#include <glm/glm.hpp>

namespace bird {

struct alignas(16) Quad {
  glm::mat3x2 transform;
  uint32_t material_index;

  uint32_t _padding; // so the last 4 bytes are zeroed out
};

} // bird