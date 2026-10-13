#pragma once

#include <unordered_map>
#include <memory>
#include <cstdint>

#include "vulkan_pipeline.hpp"

namespace bird {

class VulkanPipelineCache {
 public:
  explicit VulkanPipelineCache(VulkanSwapchain& swapchain, VulkanContext& vk_context, VulkanMaterialHandler& material_handler, vk::raii::DescriptorSetLayout& descriptor_set_layout)
      : swapchain(swapchain), vk_context(vk_context), material_handler(material_handler), descriptor_set_layout(descriptor_set_layout) {};
  ~VulkanPipelineCache() = default;

  Result<VulkanPipeline*> get(const AssetHandle<VertexShaderAsset>& vert, const AssetHandle<FragmentShaderAsset>& frag);
 private:
  struct PipelineKey {
    uint64_t vert_id;
    uint64_t frag_id;

    bool operator==(const PipelineKey& other) const {
      return vert_id == other.vert_id && frag_id == other.frag_id;
    }
  };

  struct PipelineKeyHash {
    std::size_t operator()(const PipelineKey& k) const {
      return std::hash<uint64_t>()(k.vert_id) ^ (std::hash<uint64_t>()(k.frag_id) << 1);
    }
  };

  std::unordered_map<PipelineKey, std::unique_ptr<VulkanPipeline>, PipelineKeyHash> pipelines;

  VulkanSwapchain& swapchain;
  VulkanContext& vk_context;
  VulkanMaterialHandler& material_handler;
  vk::raii::DescriptorSetLayout& descriptor_set_layout;
};

} // bird