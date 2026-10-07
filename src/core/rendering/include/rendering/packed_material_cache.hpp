#pragma once

#include "rendering/rendering_context.hpp"
#include "resources/resources_context.hpp"
#include "rendering/packed_material.hpp"
#include "rendering/material_shader_layout.hpp"
#include "resources/material_types.hpp"
#include "utils/result.hpp"

namespace bird {

class PackedMaterialCache {
 public:
  explicit PackedMaterialCache(RenderingContext& context)
  : material_shader_layout_registry(context.material_shader_layout_registry),
  material_packer(context.material_packer),
  material_pool(context.resources_context->material_pool) {}
  ~PackedMaterialCache() = default;

  Result<PackedMaterial> get(MaterialID material_id);
 private:
  struct CacheEntry {
    PackedMaterial packed_data;
    uint32_t generation = 0;
    uint32_t version = 0;
    bool is_valid = false;
  };

  std::vector<CacheEntry> cache;

  MaterialShaderLayoutRegistry* material_shader_layout_registry;
  MaterialPacker* material_packer;
  MaterialPool* material_pool;
};

} // bird