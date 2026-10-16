#pragma once

namespace bird {

struct ResourcesContext;

class SceneFactory;
class RenderGatherer;

/**
 * @brief Context for the node system layer.
 */
struct NodeSystemContext {
  ResourcesContext* resources_context;

  SceneFactory* scene_factory;
  RenderGatherer* render_gatherer;
};

} // bird