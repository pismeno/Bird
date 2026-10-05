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

class Material {
 public:
  Material() = default;

  void set_property(std::string_view name, const MaterialProperty& value);

  template <typename T>
  [[nodiscard]] Result<T> get_property(const std::string_view name) const {
    auto it = properties.find(name);
    if (it == properties.end()) {
      return bird::fail("Material property '" + std::string(name) + "' not found.");
    }

    const T* val = std::get_if<T>(&it->second);
    if (!val) {
      return bird::fail("Material property '" + std::string(name) + "' exists, but is a different type.");
    }

    return *val;
  }

  void clear_properties();

 private:
  std::unordered_map<std::string, MaterialProperty, StringHash, std::equal_to<>> properties;
};

} // bird