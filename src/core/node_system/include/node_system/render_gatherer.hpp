#pragma once

#include <resources/render_command.hpp>
#include <node_system/scene.hpp>

namespace bird {

class RenderGatherer {
 public:
  explicit RenderGatherer() = default;

  /**
   * @brief Generates render commands for the given scene.
   * @param scene reference to the scene
   * @return vector of generated render commands
   */
  std::vector<RenderCommand> gather_render_commands(Scene& scene);
};

} // bird