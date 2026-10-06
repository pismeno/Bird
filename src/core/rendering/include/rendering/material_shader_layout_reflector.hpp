#pragma once

#include <cstdint>

#include "rendering/material_shader_layout.hpp"
#include "resources/asset_types.hpp"
#include "resources/asset_handle.hpp"
#include "utils/result.hpp"

namespace bird {

class MaterialShaderLayoutReflector {
 public:
  explicit MaterialShaderLayoutReflector() = default;
  ~MaterialShaderLayoutReflector() = default;

  Result<MaterialShaderLayout> reflect(const uint32_t* spirv_words, size_t size_bytes) const;
  inline Result<MaterialShaderLayout> reflect(const AssetHandle<ShaderAsset>& shader) const { return reflect(shader->as_32bit_words(), shader->get_size_bytes()); }
};

} // bird