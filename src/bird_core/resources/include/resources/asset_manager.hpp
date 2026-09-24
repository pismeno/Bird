#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <memory>

#include <resources/asset_types.hpp>
#include <resources/resources_context.hpp>
#include <resources/asset_handle.hpp>
#include <utils/string_hash.hpp>
#include <utils/result.hpp>
#include "resources/asset_pool.hpp"

namespace bird {

class AssetManager {

  template <typename> friend class AssetHandle;

 public:
  template <std::derived_from<ShaderAsset> T, typename... Args>
  Result<AssetHandle<T>> acquire(const std::string& filepath, Args&&... args) { // acquire implementation for shader assets

    std::string expected_suffix = std::string(".") + T::EXTENSION + ".spv";

    if (!filepath.ends_with(expected_suffix)) {
      return bird::fail("Requested shader type's file must have the suffix '" + expected_suffix + "'");
    }

    return get_pool<T>()->load(filepath, std::forward<Args>(args)...);
  }

  template <typename T, typename... Args>
  requires (!std::derived_from<T, ShaderAsset>)
  Result<AssetHandle<T>> acquire(const std::string& filepath, Args&&... args) {
    return get_pool<T>()->load(filepath, std::forward<Args>(args)...);
  }

  static Result<std::unique_ptr<AssetManager>> create(ResourcesContext& context);
 private:
  AssetManager() = default;
  Result<void> init(ResourcesContext& context);

  template <typename T>
  AssetPool<T>* get_pool() {
    auto it = asset_pools.find(T::TYPE_ID);

    if (it == asset_pools.end()) {
      it = asset_pools.emplace(T::TYPE_ID, std::make_unique<AssetPool<T>>(file_system)).first;
    }

    return static_cast<AssetPool<T>*>(it->second.get());
  }

  std::unordered_map<std::string, std::unique_ptr<IAssetPool>, StringHash, std::equal_to<>> asset_pools;

  IFileSystem* file_system = nullptr;
};

} // bird
