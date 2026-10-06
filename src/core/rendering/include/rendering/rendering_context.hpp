#pragma once

namespace bird {

struct ResourcesContext;
class MaterialShaderLayoutReflector;
class MaterialShaderLayoutRegistry;
class BindlessTextureRegistry;
class IRenderer;

struct RenderingContext {
  ResourcesContext* resources_context;

  MaterialShaderLayoutReflector* material_shader_layout_reflector;
  MaterialShaderLayoutRegistry* material_shader_layout_registry;
  BindlessTextureRegistry* bindless_texture_registry;
  IRenderer* renderer;
};

} // bird