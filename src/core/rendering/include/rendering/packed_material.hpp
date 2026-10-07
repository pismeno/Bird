#pragma once

#include <vector>

namespace bird {

struct PackedMaterial {
  std::vector<std::byte> data;
};

} // bird