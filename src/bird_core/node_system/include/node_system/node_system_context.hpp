#pragma once

namespace bird {

struct ResourcesContext;

class SceneFactory;

struct NodeSystemContext {
  ResourcesContext* resources_context;

  SceneFactory* scene_factory;
};

} // bird