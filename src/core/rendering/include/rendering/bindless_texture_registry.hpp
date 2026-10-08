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

  /**
   * @brief Get the bindless index for the given texture. If the texture doesn't have an index it allocates a new one for it,
   * and creates a pending texture upload for it.
   */
  uint32_t get_index(const AssetHandle<ImageAsset>& texture);

  /**
   * @brief Get all pending texture uploads.
   */
  [[nodiscard]] std::vector<AssetHandle<ImageAsset>> get_pending_texture_uploads() const;

  /**
   * @brief Clear all pending texture uploads. Should be done after resolving all pending texture uploads.
   */
  void clear_pending_texture_uploads();
 private:
  uint32_t allocate_index(const AssetHandle<ImageAsset>& texture);

  std::unordered_map<uint64_t, uint32_t> bindless_indices;
  std::unordered_map<uint64_t, AssetHandle<ImageAsset>> pending_texture_uploads;

  uint32_t next_unused_index {0};
};

} // bird