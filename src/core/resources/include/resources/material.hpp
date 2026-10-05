#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <unordered_map>

#include <glm/glm.hpp>

#include "utils/result.hpp"
#include "utils/string_hash.hpp"
#include "resources/asset_handle.hpp"
#include "resources/asset_types.hpp"

namespace bird {

using MaterialProperty = std::variant<
    float,
    int,
    glm::vec2,
    glm::vec3,
    glm::vec4,
    AssetHandle<ImageAsset>
    >;

struct MaterialData {
  std::unordered_map<std::string, MaterialProperty, StringHash, std::equal_to<>> properties;

  AssetHandle<VertexShaderAsset> vertex_shader;
  AssetHandle<FragmentShaderAsset> fragment_shader;
};

} // bird