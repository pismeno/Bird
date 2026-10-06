#include "rendering/material_shader_layout_registry.hpp"

namespace bird {

Result<MaterialShaderLayout> MaterialShaderLayoutRegistry::get(const AssetHandle<ShaderAsset>& shader) {
  uint64_t shader_id = shader.get_packed_id();
  auto it = material_shader_layouts.find(shader_id);
  if (it != material_shader_layouts.end()) {
    return it->second;
  }

  auto r_reflect = material_shader_layout_reflector->reflect(shader);
  if (!r_reflect) return r_reflect;

  material_shader_layouts[shader_id] = std::move(r_reflect).value();

  return material_shader_layouts[shader_id];
}

} // bird