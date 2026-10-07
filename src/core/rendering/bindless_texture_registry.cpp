#include "rendering/bindless_texture_registry.hpp"

#include <cstdint>

namespace bird {

uint32_t BindlessTextureRegistry::get_index(uint64_t texture_id) {
  auto it = bindless_indices.find(texture_id);
  if (it != bindless_indices.end()) {
    return it->second;
  }

  uint32_t bindless_index = next_unused_index;
  next_unused_index++;
  bindless_indices[texture_id] = bindless_index;
  return bindless_index;
}

} // bird