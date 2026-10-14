#pragma once

#include <unordered_map>
#include <memory>
#include <cstdint>

#include "vulkan_pipeline.hpp"
#include "combined_shader_key.hpp"

namespace bird {

class VulkanPipelineCache {
 public:
  explicit VulkanPipelineCache(VulkanSwapchain& swapchain, VulkanContext& vk_context, VulkanMaterialHandler& material_handler, vk::raii::DescriptorSetLayout& descriptor_set_layout)
      : swapchain(swapchain), vk_context(vk_context), material_handler(material_handler), descriptor_set_layout(descriptor_set_layout) {};
  ~VulkanPipelineCache() = default;

  Result<VulkanPipeline*> get(const AssetHandle<VertexShaderAsset>& vert, const AssetHandle<FragmentShaderAsset>& frag);
 private:
  std::unordered_map<CombinedShaderKey, std::unique_ptr<VulkanPipeline>, CombinedShaderKeyHash> pipelines;

  VulkanSwapchain& swapchain;
  VulkanContext& vk_context;
  VulkanMaterialHandler& material_handler;
  vk::raii::DescriptorSetLayout& descriptor_set_layout;
};

} // bird