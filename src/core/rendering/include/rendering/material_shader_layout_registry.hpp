#pragma once

#include <unordered_map>

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

  Result<MaterialShaderLayout> get(const AssetHandle<ShaderAsset>& shader);
 private:
  std::unordered_map<uint64_t, MaterialShaderLayout> material_shader_layouts;

  MaterialShaderLayoutReflector* material_shader_layout_reflector;
};

} // bird