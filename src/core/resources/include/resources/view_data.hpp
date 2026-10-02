#pragma once

#include <glm/glm.hpp>

namespace bird {

/**
 * @brief Contains view and projection matrices. Can be used as a camera.
 */
struct alignas(16) ViewData {
  glm::mat4 view;
  glm::mat4 projection;

  /// ViewProjection = View * Projection
  glm::mat4 viewProjection;
};

} // bird