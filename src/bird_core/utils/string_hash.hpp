#pragma once

#include <string_view>
#include <functional>

namespace bird {

struct StringHash {
  using is_transparent = void; // enables C++20 heterogeneous lookup

  [[nodiscard]] size_t operator()(std::string_view txt) const noexcept {
    return std::hash<std::string_view>{}(txt);
  }
};

}