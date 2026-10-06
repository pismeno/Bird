#pragma once

#include <vector>

#include "rendering/rendering_context.hpp"
#include "rendering/material_shader_layout.hpp"
#include "rendering/bindless_texture_registry.hpp"
#include "resources/material.hpp"

namespace bird {

class MaterialBaker {
 public:
  explicit MaterialBaker(RenderingContext& rendering_context) : bindless_texture_registry(rendering_context.bindless_texture_registry) {}
  ~MaterialBaker() = default;

  std::vector<std::byte> bake(const Material& material, const MaterialShaderLayout& layout) const;
 private:
  BindlessTextureRegistry* bindless_texture_registry;
};

} // bird