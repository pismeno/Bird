#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <memory>

#include <resources/asset_handle.hpp>
#include <utils/string_hash.hpp>
#include "resources/asset_pool.hpp"

namespace bird::resources {

struct BinaryAsset {
  alignas(8) std::vector<uint8_t> data;
};

struct ShaderAsset : public BinaryAsset { static constexpr const char* TYPE_ID = "shader_asset"; };
struct GenericBinaryAsset : public BinaryAsset { static constexpr const char* TYPE_ID = "generic_binary_asset"; };

class BinaryAssetManager {

  template <typename> friend class AssetHandle;

 public:
  template <typename T>
  AssetHandle<T> acquire(const std::string& filepath) {
    return get_pool<T>()->load(filepath);
  }

 private:
  template <typename T>
  AssetPool<T>* get_pool() {
    auto it = asset_pools.find(T::TYPE_ID);

    if (it == asset_pools.end()) {
      it = asset_pools.emplace(T::TYPE_ID, std::make_unique<AssetPool<T>>()).first;
    }

    return static_cast<AssetPool<T>*>(it->second.get());
  }

  std::unordered_map<std::string, std::unique_ptr<IAssetPool>, StringHash, std::equal_to<>> asset_pools;
};

} // bird::resources
