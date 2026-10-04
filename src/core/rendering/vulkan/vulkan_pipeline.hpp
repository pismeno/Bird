#pragma once

#include <memory>

#include <vulkan/vulkan_raii.hpp>

#include "utils/result.hpp"
#include "resources/asset_types.hpp"
#include "rendering/vulkan/vulkan_context.hpp"
#include "rendering/vulkan/vulkan_swapchain.hpp"
#include "resources/asset_manager.hpp"

namespace bird {

class VulkanPipeline {
 public:
  ~VulkanPipeline() = default;

  static Result<std::unique_ptr<VulkanPipeline>> create(VulkanSwapchain& swapchain, VulkanContext& vk_context, AssetManager* asset_manager);
 private:
  explicit VulkanPipeline() = default;

  Result<void> create_descriptor_set_layout(VulkanContext& vk_context);
  Result<void> create_graphics_pipeline(VulkanSwapchain& swapchain, VulkanContext& vk_context, AssetManager* asset_manager);

  vk::raii::DescriptorSetLayout descriptor_set_layout = nullptr;
  vk::raii::PipelineLayout pipeline_layout = nullptr;
  vk::raii::Pipeline graphics_pipeline = nullptr;
};

} // bird