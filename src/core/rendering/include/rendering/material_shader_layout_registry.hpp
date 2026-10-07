#pragma once

#include <unordered_map>
#include <concepts>

#include "rendering/material_shader_layout.hpp"
#include "rendering/rendering_context.hpp"
#include "rendering/material_shader_layout_reflector.hpp"
#include "resources/resources_context.hpp"
#include "resources/material_pool.hpp"
#include "resources/material_types.hpp"
#include "utils/result.hpp"

namespace bird {

class MaterialShaderLayoutRegistry {
 public:
  explicit MaterialShaderLayoutRegistry(const RenderingContext& context)
  : material_shader_layout_reflector(context.material_shader_layout_reflector) {};
  ~MaterialShaderLayoutRegistry() = default;

  template <typename A>
  requires (std::derived_from<A, ShaderAsset>)
  Result<MaterialShaderLayout> get(const AssetHandle<A>& shader) {
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
 private:
  std::unordered_map<uint64_t, MaterialShaderLayout> material_shader_layouts;

  MaterialShaderLayoutReflector* material_shader_layout_reflector;
};

} // bird