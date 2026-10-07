#pragma once

#include <vector>

#include "rendering/rendering_context.hpp"
#include "rendering/material_shader_layout.hpp"
#include "rendering/bindless_texture_registry.hpp"
#include "resources/material.hpp"

namespace bird {

class MaterialPacker {
 public:
  explicit MaterialPacker(RenderingContext& rendering_context) : bindless_texture_registry(rendering_context.bindless_texture_registry) {}
  ~MaterialPacker() = default;

  std::vector<std::byte> pack(const Material& material, const MaterialShaderLayout& layout) const;
 private:
  BindlessTextureRegistry* bindless_texture_registry;
};

} // bird