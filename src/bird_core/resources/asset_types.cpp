#include <resources/asset_types.hpp>

#include <string>
#include <string_view>
#include <cstdint>

namespace bird {

[[nodiscard]] const uint32_t* ShaderAsset::as_32bit_words() const noexcept {
  return reinterpret_cast<const uint32_t*>(data.data());
}

[[nodiscard]] std::string_view TextAsset::as_string_view() const noexcept {
  if (data.empty()) return {};

  return std::string_view(reinterpret_cast<const char*>(data.data()), data.size());
}

[[nodiscard]] std::string TextAsset::as_string() const {
  return std::string(as_string_view());
}

} // bird