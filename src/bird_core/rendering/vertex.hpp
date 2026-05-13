#pragma once

#include <cstdint>

struct Vertex {
    float x, y;
    float u, v;
    std::uint32_t color = 0xffffffffu;
    std::uint32_t control = 0; // 1-4 bits for texture index, 5-8 font sdf control, 9-32 reserved for future use
};
