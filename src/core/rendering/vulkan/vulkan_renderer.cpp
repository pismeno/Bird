#include "rendering/vulkan/vulkan_renderer.hpp"

#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <string>
#include <memory>
#include <iostream>

#include <rendering/vulkan/vertex.hpp>
#include <rendering/vulkan/vulkan_context.hpp>
#include <rendering/vulkan/vulkan_memory_allocator.hpp>
#include <rendering/vulkan/vulkan_swapchain.hpp>
#include <rendering/vulkan/vulkan_pipeline.hpp>
#include <resources/asset_handle.hpp>
#include <resources/asset_types.hpp>
#include <utils/result.hpp>

namespace bird {

VulkanRenderer::~VulkanRenderer() {
  if (!is_context_lost_ && vk_context) {
    vk_context->get_logical_device().waitIdle();
  }
}

Result<std::unique_ptr<VulkanRenderer>> VulkanRenderer::create(RenderingContext& context, void* native_window_handle, uint32_t window_width, uint32_t window_height) {
  std::unique_ptr<VulkanRenderer> renderer(new VulkanRenderer());

  renderer->asset_manager = context.resources_context->asset_manager;

  // 1. Initialize Subsystems
  auto r_context = VulkanContext::create(native_window_handle);
  if (!r_context) return bird::fail(r_context.error());
  renderer->vk_context = std::move(r_context).value();
  std::cout << "Vulkan Context created successfully" << std::endl;

  auto r_allocator = VulkanMemoryAllocator::create(*renderer->vk_context);
  if (!r_allocator) return bird::fail(r_allocator.error());
  renderer->memory_allocator = std::move(r_allocator).value();
  std::cout << "VMA Allocator created successfully" << std::endl;

  auto r_swapchain = VulkanSwapchain::create(*renderer->vk_context, window_width, window_height);
  if (!r_swapchain) return bird::fail(r_swapchain.error());
  renderer->swapchain = std::move(r_swapchain).value();
  std::cout << "Swapchain created successfully" << std::endl;

  // 2. Initialize Internal Renderer State
  auto r_create_command_pool = renderer->create_command_pool();
  if (!r_create_command_pool) return bird::fail(r_create_command_pool.error());

  auto r_material_handler = VulkanMaterialHandler::create(*renderer->vk_context, context);
  if (!r_material_handler) return bird::fail(r_material_handler.error());
  renderer->material_handler = std::move(r_material_handler).value();
  std::cout << "Material Handler created successfully" << std::endl;

  auto r_pipeline = VulkanPipeline::create(*renderer->swapchain, *renderer->vk_context, *renderer->material_handler, renderer->asset_manager);
  if (!r_pipeline) return bird::fail(r_pipeline.error());
  renderer->pipeline = std::move(r_pipeline).value();
  std::cout << "Pipeline created successfully" << std::endl;

  // 3. Initialize Assets
  auto r_create_texture_image = renderer->create_texture_image();
  if (!r_create_texture_image) return bird::fail(r_create_texture_image.error());

  auto r_create_texture_image_view = renderer->create_texture_image_view();
  if (!r_create_texture_image_view) return bird::fail(r_create_texture_image_view.error());

  auto r_create_texture_sampler = renderer->create_texture_sampler();
  if (!r_create_texture_sampler) return bird::fail(r_create_texture_sampler.error());

  auto r_create_vertex_buffer = renderer->create_vertex_buffer();
  if (!r_create_vertex_buffer) return bird::fail(r_create_vertex_buffer.error());

  auto r_create_index_buffer = renderer->create_index_buffer();
  if (!r_create_index_buffer) return bird::fail(r_create_index_buffer.error());

  auto r_create_frame_data = renderer->create_frame_data();
  if (!r_create_frame_data) return bird::fail(r_create_frame_data.error());

  std::cout << "Vulkan Renderer initialized successfully" << std::endl;

  return std::move(renderer);
}

Result<void> VulkanRenderer::create_command_pool() {
  vk::CommandPoolCreateInfo pool_info{
      .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = vk_context->get_graphics_queue_index()
  };

  auto vkr_create_command_pool = vk_context->get_logical_device().createCommandPool(pool_info);
  if (vkr_create_command_pool.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create command pool");
  }

  command_pool = std::move(vkr_create_command_pool.value);
  return bird::ok();
}

Result<void> VulkanRenderer::create_frame_data() {
  frames.resize(MAX_FRAMES_IN_FLIGHT);

  // 1. Create Descriptor Pool for UBOs (Set 0)
  vk::DescriptorPoolSize pool_size{
      .type            = vk::DescriptorType::eUniformBuffer,
      .descriptorCount = MAX_FRAMES_IN_FLIGHT
  };

  vk::DescriptorPoolCreateInfo pool_info{
      .flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets       = MAX_FRAMES_IN_FLIGHT,
      .poolSizeCount = 1,
      .pPoolSizes    = &pool_size
  };

  auto vkr_ubo_pool = vk_context->get_logical_device().createDescriptorPool(pool_info);
  if (vkr_ubo_pool.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create UBO descriptor pool");
  }
  ubo_descriptor_pool = std::move(vkr_ubo_pool.value);

  // 2. Allocate Command Buffers
  vk::CommandBufferAllocateInfo alloc_info{
      .commandPool        = *command_pool,
      .level              = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = MAX_FRAMES_IN_FLIGHT
  };

  auto vkr_cmd_buffers = vk_context->get_logical_device().allocateCommandBuffers(alloc_info);
  if (vkr_cmd_buffers.result != vk::Result::eSuccess) {
    return bird::fail("Failed to allocate command buffers");
  }

  // 3. Initialize Per-Frame Data
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    FrameData& frame = frames[i];

    // Sync Objects
    auto vkr_image_avail = vk_context->get_logical_device().createSemaphore(vk::SemaphoreCreateInfo());
    if (vkr_image_avail.result != vk::Result::eSuccess) return bird::fail("Failed to create semaphore");
    frame.image_available_semaphore = std::move(vkr_image_avail.value);

    auto vkr_fence = vk_context->get_logical_device().createFence(vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
    if (vkr_fence.result != vk::Result::eSuccess) return bird::fail("Failed to create fence");
    frame.in_flight_fence = std::move(vkr_fence.value);

    // Command Buffer
    frame.command_buffer = std::move(vkr_cmd_buffers.value[i]);

    // Uniform Buffer
    auto ubo_res = memory_allocator->create_buffer(vk::BufferUsageFlagBits::eUniformBuffer, sizeof(ViewData));
    if (!ubo_res) return bird::fail(ubo_res.error());
    frame.uniform_buffer = std::move(ubo_res).value();
    frame.uniform_buffer_mapped = frame.uniform_buffer.getAllocation().getInfo().pMappedData;

    // Allocate Set 0 for this frame
    vk::DescriptorSetLayout raw_ubo_layout = *pipeline->get_descriptor_set_layout();
    vk::DescriptorSetAllocateInfo desc_alloc_info{
        .descriptorPool     = *ubo_descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts        = &raw_ubo_layout
    };

    auto vkr_desc_set = vk_context->get_logical_device().allocateDescriptorSets(desc_alloc_info);
    if (vkr_desc_set.result != vk::Result::eSuccess) {
      return bird::fail("Failed to allocate frame UBO descriptor set");
    }
    frame.descriptor_set = std::move(vkr_desc_set.value[0]);

    // Point Set 0 to this frame's uniform buffer
    vk::DescriptorBufferInfo buffer_info{
        .buffer = *frame.uniform_buffer,
        .offset = 0,
        .range  = sizeof(ViewData)
    };

    vk::WriteDescriptorSet write_desc{
        .dstSet          = *frame.descriptor_set,
        .dstBinding      = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType  = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo     = &buffer_info
    };

    vk_context->get_logical_device().updateDescriptorSets(write_desc, nullptr);
  }

  // 4. Render Finished Semaphores (Tied to Swapchain image count)
  render_finished_semaphores.clear();
  for (size_t i = 0; i < swapchain->get_image_count(); i++) {
    auto vkr_render_fin = vk_context->get_logical_device().createSemaphore(vk::SemaphoreCreateInfo());
    if (vkr_render_fin.result != vk::Result::eSuccess) return bird::fail("Failed to create render finished semaphore");
    render_finished_semaphores.push_back(std::move(vkr_render_fin.value));
  }

  return bird::ok();
}

Result<void> VulkanRenderer::create_texture_image() {
  auto r_load_img = asset_manager->acquire<ImageAsset>("editor://test_img.png");
  if (!r_load_img) return bird::fail(r_load_img.error());
  AssetHandle<ImageAsset> img_asset = std::move(r_load_img).value();

  auto r_staging_buffer = memory_allocator->create_buffer(
      vk::BufferUsageFlagBits::eTransferSrc,
      std::span(img_asset->pixel_data)
  );

  if (!r_staging_buffer) return bird::fail(r_staging_buffer.error());
  vma::raii::Buffer staging_buffer = std::move(r_staging_buffer).value();

  vk::ImageCreateInfo image_info{
      .imageType     = vk::ImageType::e2D,
      .format        = vk::Format::eR8G8B8A8Srgb,
      .extent        = { static_cast<uint32_t>(img_asset->width), static_cast<uint32_t>(img_asset->height), 1 },
      .mipLevels     = 1,
      .arrayLayers   = 1,
      .samples       = vk::SampleCountFlagBits::e1,
      .tiling        = vk::ImageTiling::eOptimal,
      .usage         = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
      .sharingMode   = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined
  };

  vma::AllocationCreateInfo image_alloc_info{
      .flags = vma::AllocationCreateFlagBits::eDedicatedMemory,
      .usage = vma::MemoryUsage::eAuto
  };

  auto vkr_create_image = memory_allocator->get_allocator().createImage(image_info, image_alloc_info);
  if (vkr_create_image.result != vk::Result::eSuccess) return bird::fail("Failed to create texture image");
  texture_image = std::move(vkr_create_image.value);

  // Command recording
  auto r_cmd_buffer = begin_single_time_commands();
  if (!r_cmd_buffer) return bird::fail(r_cmd_buffer.error());
  vk::raii::CommandBuffer cmd_buffer = std::move(r_cmd_buffer).value();

  transition_image_layout(cmd_buffer, texture_image,
                          vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                          {}, vk::AccessFlagBits2::eTransferWrite,
                          vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eTransfer);

  vk::BufferImageCopy region{
      .imageSubresource = { .aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 },
      .imageExtent      = {static_cast<uint32_t>(img_asset->width), static_cast<uint32_t>(img_asset->height), 1}
  };

  cmd_buffer.copyBufferToImage(*staging_buffer, *texture_image, vk::ImageLayout::eTransferDstOptimal, region);

  transition_image_layout(cmd_buffer, texture_image,
                          vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
                          vk::AccessFlagBits2::eTransferWrite, vk::AccessFlagBits2::eShaderRead,
                          vk::PipelineStageFlagBits2::eTransfer, vk::PipelineStageFlagBits2::eFragmentShader);

  end_single_time_commands(cmd_buffer);
  return bird::ok();
}

Result<void> VulkanRenderer::create_texture_image_view() {
  vk::ImageViewCreateInfo view_info{
      .image            = *texture_image,
      .viewType         = vk::ImageViewType::e2D,
      .format           = vk::Format::eR8G8B8A8Srgb,
      .subresourceRange = { .aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1 }
  };

  auto vkr_create_view = vk_context->get_logical_device().createImageView(view_info);
  if (vkr_create_view.result != vk::Result::eSuccess) return bird::fail("Failed to create texture image view");
  texture_image_view = std::move(vkr_create_view.value);
  return bird::ok();
}

Result<void> VulkanRenderer::create_texture_sampler() {
  vk::SamplerCreateInfo sampler_info{
      .magFilter               = vk::Filter::eNearest,
      .minFilter               = vk::Filter::eLinear,
      .mipmapMode              = vk::SamplerMipmapMode::eLinear,
      .addressModeU            = vk::SamplerAddressMode::eRepeat,
      .addressModeV            = vk::SamplerAddressMode::eRepeat,
      .addressModeW            = vk::SamplerAddressMode::eRepeat,
      .maxAnisotropy           = 1.0f,
      .compareOp               = vk::CompareOp::eAlways,
      .borderColor             = vk::BorderColor::eIntOpaqueBlack,
  };

  auto vkr_create_sampler = vk_context->get_logical_device().createSampler(sampler_info);
  if (vkr_create_sampler.result != vk::Result::eSuccess) return bird::fail("Failed to create texture sampler");
  texture_sampler = std::move(vkr_create_sampler.value);
  return bird::ok();
}

Result<void> VulkanRenderer::create_vertex_buffer() {
  auto r_buf = memory_allocator->create_buffer(vk::BufferUsageFlagBits::eVertexBuffer, std::span(vertices));
  if (!r_buf) return bird::fail(r_buf.error());
  vertex_buffer = std::move(r_buf).value();
  return bird::ok();
}

Result<void> VulkanRenderer::create_index_buffer() {
  auto r_buf = memory_allocator->create_buffer(vk::BufferUsageFlagBits::eIndexBuffer, std::span(indices));
  if (!r_buf) return bird::fail(r_buf.error());
  index_buffer = std::move(r_buf).value();
  return bird::ok();
}

Result<void> VulkanRenderer::record_command_buffer(FrameData& frame) {
  if (frame.command_buffer.begin({}) != vk::Result::eSuccess) return bird::fail("Failed to begin command buffer");

  transition_image_layout(frame.command_buffer, swapchain->get_images()[current_image_index_],
                          vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
                          {}, vk::AccessFlagBits2::eColorAttachmentWrite,
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eColorAttachmentOutput);

  vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 1.0f, 0.5f, 1.0f);
  vk::RenderingAttachmentInfo attachmentInfo = {
      .imageView   = *swapchain->get_image_views()[current_image_index_],
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp      = vk::AttachmentLoadOp::eClear,
      .storeOp     = vk::AttachmentStoreOp::eStore,
      .clearValue  = clearColor};

  vk::RenderingInfo renderingInfo = {
      .renderArea           = {.offset = {0, 0}, .extent = swapchain->get_extent()},
      .layerCount           = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments    = &attachmentInfo};

  frame.command_buffer.beginRendering(renderingInfo);

  pipeline->bind(frame.command_buffer);

  std::array<vk::DescriptorSet, 2> sets = {
      *frame.descriptor_set,                               // Set 0: UBO
      material_handler->get_texture_descriptor_set()       // Set 1: Bindless textures
  };

  frame.command_buffer.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics,
      *pipeline->get_layout(),
      0, // firstSet = 0
      sets,
      nullptr
  );

  frame.command_buffer.bindVertexBuffers(0, *vertex_buffer, {0});
  frame.command_buffer.bindIndexBuffer(*index_buffer, 0, vk::IndexType::eUint16);
  frame.command_buffer.setViewport(0, vk::Viewport(0.0f, static_cast<float>(swapchain->get_extent().height), static_cast<float>(swapchain->get_extent().width), -static_cast<float>(swapchain->get_extent().height), 0.0f, 1.0f));
  frame.command_buffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain->get_extent()));

  frame.command_buffer.drawIndexed(indices.size(), 1, 0, 0, 0);

  frame.command_buffer.endRendering();

  transition_image_layout(frame.command_buffer, swapchain->get_images()[current_image_index_],
                          vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
                          vk::AccessFlagBits2::eColorAttachmentWrite, {},
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe);

  if (frame.command_buffer.end() != vk::Result::eSuccess) return bird::fail("Failed to end command buffer");

  return bird::ok();
}

void VulkanRenderer::begin_frame() {
  if (needs_framebuffer_resize_ || is_context_lost_) return;

  FrameData& frame = frames[current_frame_];
  auto vkr_fence = vk_context->get_logical_device().waitForFences(*frame.in_flight_fence, vk::True, UINT64_MAX);

  if (vkr_fence == vk::Result::eErrorDeviceLost) {
    is_context_lost_ = true;
    return;
  }
  vk_context->get_logical_device().resetFences(*frame.in_flight_fence);
}

void VulkanRenderer::begin_pass(const ViewData& view_data) {
  if (needs_framebuffer_resize_ || is_context_lost_) return;

  update_ubo(view_data, frames[current_frame_]);

  auto r_acquire = swapchain->acquire_next_image(frames[current_frame_].image_available_semaphore);
  if (!r_acquire) {
    needs_framebuffer_resize_ = true;
    return;
  }
  current_image_index_ = r_acquire.value();
}

void VulkanRenderer::end_pass() {
  if (needs_framebuffer_resize_ || is_context_lost_) return;
  (void) record_command_buffer(frames[current_frame_]);
}

void VulkanRenderer::end_frame() {
  if (needs_framebuffer_resize_ || is_context_lost_) return;

  FrameData& frame = frames[current_frame_];

  vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
  const vk::SubmitInfo submitInfo{
      .waitSemaphoreCount   = 1,
      .pWaitSemaphores      = &*frame.image_available_semaphore,
      .pWaitDstStageMask    = &waitDestinationStageMask,
      .commandBufferCount   = 1,
      .pCommandBuffers      = &*frame.command_buffer,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores    = &*render_finished_semaphores[current_image_index_] // FIX applied
  };

  vk_context->get_graphics_queue().submit(submitInfo, *frame.in_flight_fence);

  auto r_present = swapchain->present(vk_context->get_graphics_queue(), render_finished_semaphores[current_image_index_], current_image_index_); // FIX applied
  if (!r_present) {
    needs_framebuffer_resize_ = true;
  }

  move_to_next_frame();
}

void VulkanRenderer::update_ubo(const ViewData& view_data, FrameData& frame) {
  memcpy(frame.uniform_buffer_mapped, &view_data, sizeof(view_data));
}

Result<void> VulkanRenderer::resize_framebuffer(uint32_t width, uint32_t height) {
  vk_context->get_logical_device().waitIdle();

  auto r_recreate = swapchain->recreate(*vk_context, width, height);
  if (!r_recreate) return r_recreate;

  render_finished_semaphores.clear();
  for (size_t i = 0; i < swapchain->get_image_count(); i++) {
    auto vkr_render_fin = vk_context->get_logical_device().createSemaphore(vk::SemaphoreCreateInfo());
    if (vkr_render_fin.result != vk::Result::eSuccess) return bird::fail("Failed to create semaphore");
    render_finished_semaphores.push_back(std::move(vkr_render_fin.value));
  }

  needs_framebuffer_resize_ = false;
  return bird::ok();
}

Result<vk::raii::CommandBuffer> VulkanRenderer::begin_single_time_commands() {
  vk::CommandBufferAllocateInfo alloc_info{
      .commandPool        = *command_pool,
      .level              = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = 1
  };

  auto vkr_alloc = vk_context->get_logical_device().allocateCommandBuffers(alloc_info);
  if (vkr_alloc.result != vk::Result::eSuccess) return bird::fail("Failed to allocate command buffer");

  vk::raii::CommandBuffer command_buffer = std::move(vkr_alloc.value[0]);

  vk::CommandBufferBeginInfo begin_info{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
  if (command_buffer.begin(begin_info) != vk::Result::eSuccess) return bird::fail("Failed to begin");

  return std::move(command_buffer);
}

void VulkanRenderer::end_single_time_commands(vk::raii::CommandBuffer& command_buffer) {
  (void)command_buffer.end();

  vk::SubmitInfo submit_info{
      .commandBufferCount = 1,
      .pCommandBuffers    = &*command_buffer
  };

  vk_context->get_graphics_queue().submit(submit_info, nullptr);
  vk_context->get_graphics_queue().waitIdle();
}

void VulkanRenderer::transition_image_layout(
    vk::raii::CommandBuffer& cmd_buffer, vk::Image image,
    vk::ImageLayout old_layout, vk::ImageLayout new_layout,
    vk::AccessFlags2 src_access_mask, vk::AccessFlags2 dst_access_mask,
    vk::PipelineStageFlags2 src_stage_mask, vk::PipelineStageFlags2 dst_stage_mask)
{
  vk::ImageMemoryBarrier2 barrier = {
      .srcStageMask        = src_stage_mask,
      .srcAccessMask       = src_access_mask,
      .dstStageMask        = dst_stage_mask,
      .dstAccessMask       = dst_access_mask,
      .oldLayout           = old_layout,
      .newLayout           = new_layout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image               = image,
      .subresourceRange    = { .aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1 }
  };

  vk::DependencyInfo dependency_info = {
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers    = &barrier
  };

  cmd_buffer.pipelineBarrier2(dependency_info);
}

inline void VulkanRenderer::move_to_next_frame() {
  current_frame_ = (current_frame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

bool VulkanRenderer::is_context_lost() const noexcept { return is_context_lost_; }
bool VulkanRenderer::needs_framebuffer_resize() const noexcept { return needs_framebuffer_resize_; }

void VulkanRenderer::submit(const RenderCommand render_command) {}

} // bird