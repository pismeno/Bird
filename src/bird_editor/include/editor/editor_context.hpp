#pragma once

namespace bird {

struct NodeSystemContext;
struct RenderingContext;
struct OSContext;
struct ResourcesContext;

class ShaderCompiler;

struct EditorContext {
  NodeSystemContext* node_system_context;
  RenderingContext* rendering_context;
  OSContext* os_context;
  ResourcesContext* resources_context;

  ShaderCompiler* shader_compiler;
};

} // bird