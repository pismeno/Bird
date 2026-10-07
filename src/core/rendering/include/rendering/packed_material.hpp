#pragma once

#include <vector>

#include "resources/asset_handle.hpp"
#include "resources/asset_types.hpp"

namespace bird {

struct PackedMaterial {
  std::vector<std::byte> data;
  std::vector<AssetHandle<ImageAsset>> referenced_textures;
};

} // bird