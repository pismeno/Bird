#pragma once

#include <cstdint>
#include <array>

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace bird {

struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;
  glm::vec2 uv;
  uint32_t texture_index;

  static vk::VertexInputBindingDescription get_binding_descriptions() {
    return {.binding = 0, .stride = sizeof(Vertex), .inputRate = vk::VertexInputRate::eVertex};
  }

  static std::array<vk::VertexInputAttributeDescription, 4> get_attr_descriptions() {
    return {{
                {.location = 0, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, pos)},
                {.location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, color)},
                {.location = 2, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, uv)},
                {.location = 3, .binding = 0, .format = vk::Format::eR32Uint, .offset = offsetof(Vertex, texture_index)},
            }};
  }
};

} // bird