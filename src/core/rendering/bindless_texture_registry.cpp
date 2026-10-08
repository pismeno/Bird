#include "rendering/bindless_texture_registry.hpp"

#include <cstdint>

namespace bird {

uint32_t BindlessTextureRegistry::get_index(const AssetHandle<ImageAsset>& texture) {
  uint64_t texture_id = texture.get_packed_id();

  auto it = bindless_indices.find(texture_id);
  if (it != bindless_indices.end()) {
    return it->second;
  }

  return allocate_index(texture);
}

uint32_t BindlessTextureRegistry::allocate_index(const AssetHandle<ImageAsset>& texture) {
  uint64_t texture_id = texture.get_packed_id();

  auto it = pending_texture_uploads.find(texture_id);
  if (it == pending_texture_uploads.end()) {
    pending_texture_uploads[texture_id] = texture;
  }

  uint32_t bindless_index = next_unused_index;
  next_unused_index++;
  bindless_indices[texture_id] = bindless_index;
  return bindless_index;
}

[[nodiscard]] std::vector<AssetHandle<ImageAsset>> BindlessTextureRegistry::get_pending_texture_uploads() const {
  std::vector<AssetHandle<ImageAsset>> requests;
  requests.reserve(pending_texture_uploads.size());

  for (const auto& [key, request] : pending_texture_uploads) {
    requests.push_back(request);
  }

  return requests;
}

void BindlessTextureRegistry::clear_pending_texture_uploads() {
  pending_texture_uploads.clear();
}

} // bird