#pragma once

namespace bird {

struct ResourcesContext;

class IRenderer;

struct RenderingContext {
  ResourcesContext* resources_context;

  IRenderer* renderer;
};

} // bird