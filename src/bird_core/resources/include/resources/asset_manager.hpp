#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <memory>

#include <resources/resources_context.hpp>
#include <resources/asset_handle.hpp>
#include <utils/string_hash.hpp>
#include <utils/result.hpp>
#include "resources/asset_pool.hpp"

namespace bird {

struct BinaryAsset {
  alignas(8) std::vector<uint8_t> data;
};

struct ShaderAsset : public BinaryAsset {};

struct VertexShaderAsset : public ShaderAsset {
  static constexpr const char* TYPE_ID = "vertex_shader_asset";
  static constexpr const char* EXTENSION = "vert";
};

struct FragmentShaderAsset : public ShaderAsset {
  static constexpr const char* TYPE_ID = "fragment_shader_asset";
  static constexpr const char* EXTENSION = "frag";
};

struct ComputeShaderAsset : public ShaderAsset {
  static constexpr const char* TYPE_ID = "compute_shader_asset";
  static constexpr const char* EXTENSION = "comp";
};

struct GeometryShaderAsset : public ShaderAsset {
  static constexpr const char* TYPE_ID = "geometry_shader_asset";
  static constexpr const char* EXTENSION = "geom";
};

struct TesselationControlShaderAsset : public ShaderAsset {
  static constexpr const char* TYPE_ID = "tesselation_control_shader_asset";
  static constexpr const char* EXTENSION = "tesc";
};

struct TesselationEvaluationShaderAsset : public ShaderAsset {
  static constexpr const char* TYPE_ID = "tesselation_evaluation_shader_asset";
  static constexpr const char* EXTENSION = "tese";
};


struct GenericBinaryAsset : public BinaryAsset { static constexpr const char* TYPE_ID = "generic_binary_asset"; };

class AssetManager {

  template <typename> friend class AssetHandle;

 public:
  AssetManager(ResourcesContext& context) : context(&context) {}

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

  ResourcesContext* context;
};

} // bird
