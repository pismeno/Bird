#include "resources/material.hpp"

namespace bird {

void Material::set_property(std::string_view name, const MaterialProperty& value) {
  auto it = properties.find(name);

  if (it != properties.end()) {
    it->second = value;
  } else {
    properties.emplace(std::string(name), value);
  }
}

void Material::clear_properties() {
  properties.clear();
}

} // bird