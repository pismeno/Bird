#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>

#include "utils/string_hash.hpp"

namespace bird {

struct ShaderPropertyLayout {
  uint32_t offset;
  uint32_t size;
};

struct MaterialShaderLayout {
  std::unordered_map<std::string, ShaderPropertyLayout, StringHash, std::equal_to<>> properties;
  uint32_t total_size = 0;
};

} // bird