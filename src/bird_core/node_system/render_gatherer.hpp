#pragma once

#include "rendering/render_command.hpp"
#include "node_system/scene.hpp"

namespace bird::node_system {

class RenderGatherer {
 public:
  /**
   * @brief Generates render commands for the give scene.
   * @param scene reference to the scene
   * @return vector of generated render commands
   */
  std::vector<RenderCommand> gather_render_commands(Scene& scene);
};

} // bird::node_system