#pragma once

#include <cstdint>

#include "combined_shader_key.hpp"

namespace bird {

struct RenderBatch {
  CombinedShaderKey shader_key;
  uint32_t first_quad;
  uint32_t quad_count;
  uint32_t first_material_offset;
  uint32_t material_count;
};

} // bird