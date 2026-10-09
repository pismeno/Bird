#include "rendering/vulkan/vulkan_material_handler.hpp"
#include <cstring>

namespace bird {

Result<std::unique_ptr<VulkanMaterialHandler>> VulkanMaterialHandler::create(
    const VulkanContext& vk_context,
    VulkanMemoryAllocator& memory_allocator,
    const RenderingContext& context)
{
  std::unique_ptr<VulkanMaterialHandler> handler(new VulkanMaterialHandler(vk_context, memory_allocator, context));

  auto r_layout = handler->create_descriptor_set_layout();
  if (!r_layout) return bird::fail(r_layout.error());

  auto r_pool = handler->create_descriptor_pool();
  if (!r_pool) return bird::fail(r_pool.error());

  auto r_set = handler->create_texture_descriptor_set();
  if (!r_set) return bird::fail(r_set.error());

  auto r_sampler = handler->create_default_sampler();
  if (!r_sampler) return bird::fail(r_sampler.error());

  auto r_cmd_pool = handler->create_command_pool();
  if (!r_cmd_pool) return bird::fail(r_cmd_pool.error());

  return std::move(handler);
}

Result<void> VulkanMaterialHandler::create_descriptor_set_layout() {
  vk::DescriptorBindingFlags binding_flags =
      vk::DescriptorBindingFlagBits::eUpdateAfterBind |
      vk::DescriptorBindingFlagBits::ePartiallyBound;

  vk::DescriptorSetLayoutBindingFlagsCreateInfo binding_flags_info{
      .bindingCount  = 1,
      .pBindingFlags = &binding_flags
  };

  vk::DescriptorSetLayoutBinding binding{
      .binding         = 0,
      .descriptorType  = vk::DescriptorType::eCombinedImageSampler,
      .descriptorCount = 65536,
      .stageFlags      = vk::ShaderStageFlagBits::eFragment
  };

  vk::DescriptorSetLayoutCreateInfo create_info{
      .pNext        = &binding_flags_info,
      .flags        = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
      .bindingCount = 1,
      .pBindings    = &binding
  };

  auto vkr_create = vk_context.get_logical_device().createDescriptorSetLayout(create_info);
  if (vkr_create.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create descriptor set layout");
  }
  texture_descriptor_set_layout = std::move(vkr_create.value);
  return bird::ok();
}

Result<void> VulkanMaterialHandler::create_descriptor_pool() {
  vk::DescriptorPoolSize pool_size{vk::DescriptorType::eCombinedImageSampler, 65536};

  vk::DescriptorPoolCreateInfo create_info{
      .flags         = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind |
                       vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets       = 1,
      .poolSizeCount = 1,
      .pPoolSizes    = &pool_size
  };

  auto vkr_create = vk_context.get_logical_device().createDescriptorPool(create_info);
  if (vkr_create.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create descriptor pool");
  }
  descriptor_pool = std::move(vkr_create.value);
  return bird::ok();
}

Result<void> VulkanMaterialHandler::create_texture_descriptor_set() {
  vk::DescriptorSetLayout raw_layout = *texture_descriptor_set_layout;

  vk::DescriptorSetAllocateInfo alloc_info{
      .descriptorPool     = *descriptor_pool,
      .descriptorSetCount = 1,
      .pSetLayouts        = &raw_layout
  };

  auto vkr_create = vk_context.get_logical_device().allocateDescriptorSets(alloc_info);
  if (vkr_create.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create texture descriptor set");
  }
  bindless_texture_set = std::move(vkr_create.value[0]);
  return bird::ok();
}

Result<void> VulkanMaterialHandler::create_default_sampler() {
  vk::SamplerCreateInfo sampler_info{
      .magFilter     = vk::Filter::eLinear,
      .minFilter     = vk::Filter::eLinear,
      .mipmapMode    = vk::SamplerMipmapMode::eLinear,
      .addressModeU  = vk::SamplerAddressMode::eRepeat,
      .addressModeV  = vk::SamplerAddressMode::eRepeat,
      .addressModeW  = vk::SamplerAddressMode::eRepeat,
      .maxAnisotropy = 1.0f,
      .borderColor   = vk::BorderColor::eIntOpaqueBlack
  };

  auto vkr_sampler = vk_context.get_logical_device().createSampler(sampler_info);
  if (vkr_sampler.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create default sampler");
  }
  default_sampler = std::move(vkr_sampler.value);
  return bird::ok();
}

Result<void> VulkanMaterialHandler::create_command_pool() {
  vk::CommandPoolCreateInfo pool_info{
      .flags            = vk::CommandPoolCreateFlagBits::eTransient,
      .queueFamilyIndex = vk_context.get_graphics_queue_index()
  };

  auto vkr_pool = vk_context.get_logical_device().createCommandPool(pool_info);
  if (vkr_pool.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create transient command pool for texture uploads");
  }
  command_pool = std::move(vkr_pool.value);
  return bird::ok();
}

Result<void> VulkanMaterialHandler::update_texture_buffer() {
  std::vector<AssetHandle<ImageAsset>> pending = bindless_texture_registry->get_pending_texture_uploads();
  if (pending.empty()) {
    return bird::ok();
  }

  // 1. Compute offsets and total staging allocation size
  size_t total_staging_bytes = 0;
  std::vector<size_t> staging_offsets;
  staging_offsets.reserve(pending.size());

  uint32_t max_slot = allocated_textures.empty() ? 0 : static_cast<uint32_t>(allocated_textures.size() - 1);
  for (const auto& asset : pending) {
    staging_offsets.push_back(total_staging_bytes);
    total_staging_bytes += asset->pixel_data.size();
    max_slot = std::max(max_slot, bindless_texture_registry->get_index(asset));
  }
  allocated_textures.resize(max_slot + 1);

  // 2. Stage pixel data
  auto r_staging_buf = memory_allocator.create_buffer(vk::BufferUsageFlagBits::eTransferSrc, total_staging_bytes);
  if (!r_staging_buf) return bird::fail(r_staging_buf.error());
  vma::raii::Buffer staging_buffer = std::move(r_staging_buf).value();

  auto* staging_mapped = static_cast<uint8_t*>(staging_buffer.getAllocation().getInfo().pMappedData);
  for (size_t i = 0; i < pending.size(); ++i) {
    std::memcpy(staging_mapped + staging_offsets[i], pending[i]->pixel_data.data(), pending[i]->pixel_data.size());
  }

  // 3. Allocate and begin one-time command buffer
  auto r_cmd = allocate_upload_command_buffer();
  if (!r_cmd) return bird::fail(r_cmd.error());
  vk::raii::CommandBuffer cmd_buffer = std::move(r_cmd).value();

  // 4. Create GPU resources, prep barriers, and record transfers
  std::vector<vk::ImageMemoryBarrier2> pre_copy_barriers;
  std::vector<vk::ImageMemoryBarrier2> post_copy_barriers;
  std::vector<vk::DescriptorImageInfo> descriptor_image_infos;
  std::vector<vk::WriteDescriptorSet>  descriptor_writes;

  pre_copy_barriers.reserve(pending.size());
  post_copy_barriers.reserve(pending.size());
  descriptor_image_infos.reserve(pending.size());
  descriptor_writes.reserve(pending.size());

  for (size_t i = 0; i < pending.size(); ++i) {
    const auto& asset = pending[i];
    uint32_t slot = bindless_texture_registry->get_index(asset);

    auto r_gpu_image = create_texture_image(*asset);
    if (!r_gpu_image) return bird::fail("Failed to create GPU texture image");
    vma::raii::Image gpu_image = std::move(r_gpu_image).value();

    auto r_gpu_view = create_texture_image_view(*gpu_image);
    if (!r_gpu_view) return bird::fail("Failed to create texture image view");
    vk::raii::ImageView gpu_view = std::move(r_gpu_view).value();

    // Inlined transition: Undefined -> TransferDstOptimal
    pre_copy_barriers.push_back(vk::ImageMemoryBarrier2{
        .srcStageMask        = vk::PipelineStageFlagBits2::eTopOfPipe,
        .srcAccessMask       = {},
        .dstStageMask        = vk::PipelineStageFlagBits2::eTransfer,
        .dstAccessMask       = vk::AccessFlagBits2::eTransferWrite,
        .oldLayout           = vk::ImageLayout::eUndefined,
        .newLayout           = vk::ImageLayout::eTransferDstOptimal,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = *gpu_image,
        .subresourceRange    = { .aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1 }
    });

    // Inlined transition: TransferDstOptimal -> ShaderReadOnlyOptimal
    post_copy_barriers.push_back(vk::ImageMemoryBarrier2{
        .srcStageMask        = vk::PipelineStageFlagBits2::eTransfer,
        .srcAccessMask       = vk::AccessFlagBits2::eTransferWrite,
        .dstStageMask        = vk::PipelineStageFlagBits2::eFragmentShader,
        .dstAccessMask       = vk::AccessFlagBits2::eShaderRead,
        .oldLayout           = vk::ImageLayout::eTransferDstOptimal,
        .newLayout           = vk::ImageLayout::eShaderReadOnlyOptimal,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = *gpu_image,
        .subresourceRange    = { .aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1 }
    });

    // Staging to GPU copy command
    vk::BufferImageCopy copy_region{
        .bufferOffset      = staging_offsets[i],
        .bufferRowLength   = 0,
        .bufferImageHeight = 0,
        .imageSubresource  = { .aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 },
        .imageOffset       = { 0, 0, 0 },
        .imageExtent       = { static_cast<uint32_t>(asset->width), static_cast<uint32_t>(asset->height), 1 }
    };

    // Stash descriptor writes directly
    descriptor_image_infos.push_back({
                                         .sampler     = *default_sampler,
                                         .imageView   = *gpu_view,
                                         .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
                                     });

    descriptor_writes.push_back({
                                    .dstSet          = *bindless_texture_set,
                                    .dstBinding      = 0,
                                    .dstArrayElement = slot,
                                    .descriptorCount = 1,
                                    .descriptorType  = vk::DescriptorType::eCombinedImageSampler,
                                    .pImageInfo      = &descriptor_image_infos.back()
                                });

    allocated_textures[slot] = GpuTexture{
        .image = std::move(gpu_image),
        .view  = std::move(gpu_view)
    };
  }

  // 5. Batch-record barriers and copies
  cmd_buffer.pipelineBarrier2(vk::DependencyInfo{
      .imageMemoryBarrierCount = static_cast<uint32_t>(pre_copy_barriers.size()),
      .pImageMemoryBarriers    = pre_copy_barriers.data()
  });

  for (size_t i = 0; i < pending.size(); ++i) {
    const auto& asset = pending[i];
    uint32_t slot = bindless_texture_registry->get_index(asset);

    vk::BufferImageCopy copy_region{
        .bufferOffset      = staging_offsets[i],
        .imageSubresource  = { .aspectMask = vk::ImageAspectFlagBits::eColor, .layerCount = 1 },
        .imageExtent       = { static_cast<uint32_t>(asset->width), static_cast<uint32_t>(asset->height), 1 }
    };
    cmd_buffer.copyBufferToImage(*staging_buffer, *allocated_textures[slot].image, vk::ImageLayout::eTransferDstOptimal, copy_region);
  }

  cmd_buffer.pipelineBarrier2(vk::DependencyInfo{
      .imageMemoryBarrierCount = static_cast<uint32_t>(post_copy_barriers.size()),
      .pImageMemoryBarriers    = post_copy_barriers.data()
  });

  // 6. Submit and wait
  if (cmd_buffer.end() != vk::Result::eSuccess) return bird::fail("Failed to end upload command buffer");

  vk::SubmitInfo submit_info{ .commandBufferCount = 1, .pCommandBuffers = &*cmd_buffer };
  vk_context.get_graphics_queue().submit(submit_info, nullptr);
  vk_context.get_graphics_queue().waitIdle();

  // 7. Write descriptors and finalize
  vk_context.get_logical_device().updateDescriptorSets(descriptor_writes, nullptr);
  bindless_texture_registry->clear_pending_texture_uploads();

  return bird::ok();
}

Result<vk::raii::CommandBuffer> VulkanMaterialHandler::allocate_upload_command_buffer() {
  vk::CommandBufferAllocateInfo alloc_info{
      .commandPool        = *command_pool,
      .level              = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = 1
  };
  auto vkr = vk_context.get_logical_device().allocateCommandBuffers(alloc_info);
  if (vkr.result != vk::Result::eSuccess) return bird::fail("Failed to allocate upload command buffer");

  vk::raii::CommandBuffer cmd = std::move(vkr.value[0]);
  if (cmd.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit}) != vk::Result::eSuccess) {
    return bird::fail("Failed to begin upload command buffer");
  }
  return cmd;
}

Result<vma::raii::Image> VulkanMaterialHandler::create_texture_image(const ImageAsset& asset) {
  vk::ImageCreateInfo image_info{
      .imageType     = vk::ImageType::e2D,
      .format        = vk::Format::eR8G8B8A8Srgb,
      .extent        = { static_cast<uint32_t>(asset.width), static_cast<uint32_t>(asset.height), 1 },
      .mipLevels     = 1,
      .arrayLayers   = 1,
      .samples       = vk::SampleCountFlagBits::e1,
      .tiling        = vk::ImageTiling::eOptimal,
      .usage         = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
      .sharingMode   = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined
  };

  vma::AllocationCreateInfo alloc_info{
      .flags = vma::AllocationCreateFlagBits::eDedicatedMemory,
      .usage = vma::MemoryUsage::eAuto
  };

  auto vkr = memory_allocator.get_allocator().createImage(image_info, alloc_info);
  if (vkr.result != vk::Result::eSuccess) return bird::fail("Failed to create GPU texture image");
  return std::move(vkr.value);
}

Result<vk::raii::ImageView> VulkanMaterialHandler::create_texture_image_view(vk::Image image) {
  vk::ImageViewCreateInfo view_info{
      .image            = image,
      .viewType         = vk::ImageViewType::e2D,
      .format           = vk::Format::eR8G8B8A8Srgb,
      .subresourceRange = { .aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1 }
  };
  auto vkr = vk_context.get_logical_device().createImageView(view_info);
  if (vkr.result != vk::Result::eSuccess) return bird::fail("Failed to create texture image view");
  return std::move(vkr.value);
}

} // bird