#include "asset_pool.hpp"
#include "resources/asset_types.hpp"

#include <stb_image.h>

namespace bird {

template <>
Result<ImageAsset> AssetPool<ImageAsset>::parse_asset(std::string_view filepath, std::vector<uint8_t> raw_bytes) {
  int w, h, c;

  stbi_uc* raw_pixels = stbi_load_from_memory(
      raw_bytes.data(),
      static_cast<int>(raw_bytes.size()),
      &w, &h, &c,
      STBI_rgb_alpha
  );

  if (!raw_pixels) {
    return bird::fail("Failed to decode image data");
  }

  ImageAsset asset;
  asset.width = static_cast<uint32_t>(w);
  asset.height = static_cast<uint32_t>(h);
  asset.channels = 4;

  size_t image_size = w * h * 4;
  asset.pixel_data.assign(raw_pixels, raw_pixels + image_size);

  stbi_image_free(raw_pixels);

  return asset;
}

} // bird