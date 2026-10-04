#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace bird {

struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;
  glm::vec2 uv;

  static vk::VertexInputBindingDescription get_binding_descriptions() {
    return {.binding = 0, .stride = sizeof(Vertex), .inputRate = vk::VertexInputRate::eVertex};
  }

  static std::array<vk::VertexInputAttributeDescription, 3> get_attr_descriptions() {
    return {{
                {.location = 0, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, pos)},
                {.location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, color)},
                {.location = 2, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, uv)} // ADD THIS
            }};
  }
};

} // bird