#pragma once

#include <glm/glm.hpp>

struct RenderCommand {
  glm::mat3x3 transform;

  uint32_t texture_id;
  uint32_t z_index;
};