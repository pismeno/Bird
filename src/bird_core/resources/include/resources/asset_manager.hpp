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

/**
 * @brief Central manager for loading, caching, and retrieving assets via type-specific pools.
 */
class AssetManager {

  template <typename> friend class AssetHandle;

 public:
  /**
   * @brief Acquires a shader asset, enforcing strict file suffix matching based on the specific shader type.
   * @return Result containing a handle to the loaded shader, or an error message.
   */
  template <std::derived_from<ShaderAsset> T, typename... Args>
  Result<AssetHandle<T>> acquire(const std::string& filepath, Args&&... args) { // acquire implementation for shader assets

    std::string expected_suffix = std::string(".") + T::EXTENSION + ".spv";

    if (!filepath.ends_with(expected_suffix)) {
      return bird::fail("Requested shader type's file must have the suffix '" + expected_suffix + "'");
    }

    return get_pool<T>()->load(filepath, std::forward<Args>(args)...);
  }

  /**
   * @brief Acquires a standard (non-shader) asset.
   * @return Result containing a handle to the loaded asset, or an error message.
   */
  template <typename T, typename... Args>
  requires (!std::derived_from<T, ShaderAsset>)
  Result<AssetHandle<T>> acquire(const std::string& filepath, Args&&... args) {
    return get_pool<T>()->load(filepath, std::forward<Args>(args)...);
  }

  /**
   * @return Result containing a unique pointer to the created manager, or an error.
   */
  static Result<std::unique_ptr<AssetManager>> create(ResourcesContext& context);

 private:
  AssetManager() = default;
  Result<void> init(ResourcesContext& context);

  /**
   * @return Pointer to the type-specific AssetPool, creating it if it does not yet exist.
   */
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
