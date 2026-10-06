#pragma once

#include <cstdint>
#include <unordered_map>

#include "resources/asset_handle.hpp"
#include "resources/asset_types.hpp"

namespace bird {

class BindlessTextureRegistry {
 public:
  explicit BindlessTextureRegistry() = default;
  ~BindlessTextureRegistry() = default;

  uint32_t get_index(uint64_t texture_id);
  inline uint32_t get_index(const AssetHandle<ImageAsset>& texture) { return get_index(texture.get_packed_id()); }
 private:
  std::unordered_map<uint64_t, uint32_t> bindless_indices;

  uint32_t next_unused_index {0};
};

} // bird