#pragma once

#include <glm/glm.hpp>

#include "resources/material_types.hpp"

namespace bird {

struct RenderCommand {
  glm::mat3x3 transform;

  MaterialID material_id;
  uint32_t z_index;
};

} // bird