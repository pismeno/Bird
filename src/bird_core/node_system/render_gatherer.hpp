#pragma once

#include "rendering/render_command.hpp"
#include "node_system/scene.hpp"

namespace bird::node_system {

class RenderGatherer {
 public:
  std::vector<RenderCommand> gather_render_commands(Scene& scene);
};

} // bird::node_system