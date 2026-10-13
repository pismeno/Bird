#pragma once

#include <memory>

#include <vulkan/vulkan_raii.hpp>

#include "utils/result.hpp"
#include "rendering/vulkan/vulkan_context.hpp"
#include "rendering/vulkan/vulkan_swapchain.hpp"
#include "rendering/vulkan/vulkan_material_handler.hpp"
#include "resources/asset_manager.hpp"
#include "resources/asset_types.hpp"
#include "resources/asset_handle.hpp"

namespace bird {

struct VulkanPipelineOptions {
  VulkanSwapchain& swapchain;
  VulkanContext& vk_context;
  VulkanMaterialHandler& material_handler;
  vk::raii::DescriptorSetLayout& descriptor_set_layout;
  const AssetHandle<VertexShaderAsset>& vert_shader;
  const AssetHandle<FragmentShaderAsset>& frag_shader;
};

class VulkanPipeline {
 public:
  ~VulkanPipeline() = default;

  inline const vk::raii::PipelineLayout& get_layout() const { return pipeline_layout; }
  inline const vk::raii::Pipeline& get_pipeline() const { return graphics_pipeline; }

  void bind(const vk::raii::CommandBuffer& command_buffer) const;

  static Result<std::unique_ptr<VulkanPipeline>> create(const VulkanPipelineOptions& options);
 private:
  explicit VulkanPipeline() = default;

  Result<void> create_graphics_pipeline(const VulkanPipelineOptions& options);

  vk::raii::PipelineLayout pipeline_layout = nullptr;
  vk::raii::Pipeline graphics_pipeline = nullptr;
};

} // bird