#include "rendering/packed_material_cache.hpp"

#include "resources/material_pool.hpp"
#include "rendering/material_shader_layout_registry.hpp"
#include "rendering/material_packer.hpp"

namespace bird {

Result<PackedMaterial> PackedMaterialCache::get(MaterialID material_id) {
  if (!material_pool->is_valid(material_id)) {
    return bird::fail("MaterialID is invalid.");
  }

  uint32_t index = material_id.index();
  uint32_t pool_generation = material_id.generation();
  uint32_t pool_version = material_pool->get_version(material_id);

  if (index >= cache.size()) {
    cache.resize(index + 1);
  }

  CacheEntry& entry = cache[index];

  if (!entry.is_valid || entry.generation != pool_generation || entry.version < pool_version) {
    const Material& material = material_pool->get(material_id);

    auto r_get_layout = material_shader_layout_registry->get(material.fragment_shader);
    if (!r_get_layout) return bird::fail(r_get_layout.error());
    MaterialShaderLayout layout = std::move(r_get_layout).value();

    entry.packed_data = material_packer->pack(material, layout);
    entry.generation = pool_generation;
    entry.version = pool_version;
    entry.is_valid = true;
  }

  return entry.packed_data;
}

} // bird