#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vector>

#include "rendering/vulkan/vulkan_context.hpp"
#include "rendering/rendering_context.hpp"
#include "rendering/bindless_texture_registry.hpp"
#include "resources/material_types.hpp"
#include "utils/result.hpp"

namespace bird {

class VulkanMaterialHandler {
 public:
  ~VulkanMaterialHandler() = default;

  Result<void> update_texture_buffer();
  Result<void> prepare_materials(std::vector<MaterialID> material_ids);

  inline const vk::raii::DescriptorSetLayout& get_texture_descriptor_set_layout() const { return texture_descriptor_set_layout; }
  inline const vk::raii::DescriptorSet& get_texture_descriptor_set() const { return bindless_texture_set; }

  static Result<std::unique_ptr<VulkanMaterialHandler>> create(const VulkanContext& vk_context, const RenderingContext& context);
 private:
  explicit VulkanMaterialHandler(const RenderingContext& context) : bindless_texture_registry(context.bindless_texture_registry) {}

  Result<void> create_descriptor_set_layout(const VulkanContext& vk_context);
  Result<void> create_descriptor_pool(const VulkanContext& vk_context);
  Result<void> create_texture_descriptor_set(const VulkanContext& vk_context);

  vk::raii::DescriptorSetLayout texture_descriptor_set_layout = nullptr;
  vk::raii::DescriptorPool descriptor_pool = nullptr;
  vk::raii::DescriptorSet bindless_texture_set = nullptr;


  BindlessTextureRegistry* bindless_texture_registry;
};

} // bird