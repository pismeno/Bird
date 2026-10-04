#include "rendering/vulkan/vulkan_renderer.hpp"

#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <string>
#include <memory>

#include <resources/asset_handle.hpp>
#include <resources/asset_types.hpp>
#include <utils/result.hpp>

namespace bird {

VulkanRenderer::VulkanRenderer() {
  draw_fences.reserve(MAX_FRAMES_IN_FLIGHT);
  present_complete_semaphores.reserve(MAX_FRAMES_IN_FLIGHT);
  uniform_buffers.reserve(MAX_FRAMES_IN_FLIGHT);
  uniform_buffers_mapped.resize(MAX_FRAMES_IN_FLIGHT);
}

VulkanRenderer::~VulkanRenderer() {
  if (!is_context_lost_) {
    logical_device.waitIdle();
  }
}

Result<void> VulkanRenderer::attach_window(void* native_window_handle, uint32_t window_width, uint32_t window_height) {
  this->native_window_handle = native_window_handle;
  this->window_width = window_width;
  this->window_height = window_height;
  return bird::ok();
}

Result<void> VulkanRenderer::init(RenderingContext& context) {
  asset_manager = context.resources_context->asset_manager;

  auto r_create_instance = create_instance();
  if (!r_create_instance) return r_create_instance;
  std::cout << "Vulkan Instance created successfully" << std::endl;

  auto r_pick_physical_device = pick_physical_device();
  if (!r_pick_physical_device) return r_pick_physical_device;
  std::cout << "Vulkan Physical Device selected successfully" << std::endl;

  auto r_create_logical_device = create_logical_device();
  if (!r_create_logical_device) return r_create_logical_device;
  std::cout << "Vulkan Logical Device created successfully" << std::endl;

  auto r_create_vma = create_vma();
  if (!r_create_vma) return r_create_vma;
  std::cout << "Vulkan AMD memory allocator created successfully" << std::endl;

  auto r_create_swap_chain = create_swap_chain();
  if (!r_create_swap_chain) return r_create_swap_chain;
  std::cout << "Vulkan Swap Chain created successfully" << std::endl;

  auto r_create_image_views = create_image_views();
  if (!r_create_image_views) return r_create_image_views;
  std::cout << "Vulkan Image Views created successfully" << std::endl;

  auto r_create_descriptor_set_layout = create_descriptor_set_layout();
  if (!r_create_descriptor_set_layout) return r_create_descriptor_set_layout;
  std::cout << "Vulkan Descriptor Set Layout created successfully" << std::endl;

  auto r_create_graphics_pipeline = create_graphics_pipeline();
  if (!r_create_graphics_pipeline) return r_create_graphics_pipeline;
  std::cout << "Vulkan Graphics Pipeline created successfully" << std::endl;

  auto r_create_command_pool = create_command_pool();
  if (!r_create_command_pool) return r_create_command_pool;
  std::cout << "Vulkan Command Pool created successfully" << std::endl;

  auto r_create_texture_image = create_texture_image();
  if (!r_create_texture_image) return r_create_texture_image;
  std::cout << "Vulkan Texture Image created successfully" << std::endl;

  auto r_create_texture_image_view = create_texture_image_view();
  if (!r_create_texture_image_view) return r_create_texture_image_view;
  std::cout << "Vulkan Texture Image View created successfully" << std::endl;

  auto r_create_texture_sampler = create_texture_sampler();
  if (!r_create_texture_sampler) return r_create_texture_sampler;
  std::cout << "Vulkan Texture Sampler created successfully" << std::endl;

  auto r_create_vertex_buffer = create_vertex_buffer();
  if (!r_create_vertex_buffer) return r_create_vertex_buffer;
  std::cout << "Vulkan Vertex Buffer created successfully" << std::endl;

  auto r_create_index_buffer = create_index_buffer();
  if (!r_create_index_buffer) return r_create_index_buffer;
  std::cout << "Vulkan index Buffer created successfully" << std::endl;

  auto r_create_uniform_buffers = create_uniform_buffers();
  if (!r_create_uniform_buffers) return r_create_uniform_buffers;
  std::cout << "Vulkan Uniform Buffers created successfully" << std::endl;

  auto r_create_descriptor_pool = create_descriptor_pool();
  if (!r_create_descriptor_pool) return r_create_descriptor_pool;
  std::cout << "Vulkan Descriptor Pool created successfully" << std::endl;

  auto r_create_command_buffer = create_command_buffers();
  if (!r_create_command_buffer) return r_create_command_buffer;
  std::cout << "Vulkan Command Buffer created successfully" << std::endl;

  auto r_create_sync_objects = create_sync_objects();
  if (!r_create_sync_objects) return r_create_sync_objects;
  std::cout << "Vulkan Sync Objects created successfully" << std::endl;

  std::cout << "Vulkan Renderer initialized successfully" << std::endl;

  return bird::ok();
}

Result<void> VulkanRenderer::create_texture_image() {
  auto r_load_img = asset_manager->acquire<ImageAsset>("editor://test_img.png");
  if (!r_load_img) return bird::fail(r_load_img.error());
  AssetHandle<ImageAsset> img_asset = std::move(r_load_img).value();

  // 1. Create Staging Buffer using your existing vma::raii::Buffer helper!
  vma::raii::Buffer staging_buffer = nullptr;
  auto r_staging_buffer = create_buffer(
      vk::BufferUsageFlagBits::eTransferSrc,
      img_asset->get_size_bytes(),
      img_asset->pixel_data,
      staging_buffer
  );
  if (!r_staging_buffer) return r_staging_buffer;

  // 2. Create the Optimal vk::Image via VMA RAII
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

  auto vkr_create_image = vma_allocator.createImage(image_info, image_alloc_info);
  if (vkr_create_image.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create texture image");
  }
  texture_image = std::move(vkr_create_image.value);

  // 3. Command recording for layout transitions and copying
  auto r_cmd_buffer = begin_single_time_commands();
  if (!r_cmd_buffer) return bird::fail(r_cmd_buffer.error());

  vk::raii::CommandBuffer cmd_buffer = std::move(r_cmd_buffer).value();

  // Note the dereference operator (*) extracts the underlying raw vk::Buffer / vk::Image
  transition_image_layout(cmd_buffer, texture_image,
                          vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                          {}, vk::AccessFlagBits2::eTransferWrite,
                          vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eTransfer);

  vk::BufferImageCopy region{
      .bufferOffset      = 0,
      .bufferRowLength   = 0,
      .bufferImageHeight = 0,
      .imageSubresource  = {
          .aspectMask     = vk::ImageAspectFlagBits::eColor,
          .mipLevel       = 0,
          .baseArrayLayer = 0,
          .layerCount     = 1
      },
      .imageOffset       = {0, 0, 0},
      .imageExtent       = {img_asset->width, img_asset->height, 1}
  };

  cmd_buffer.copyBufferToImage(staging_buffer, texture_image, vk::ImageLayout::eTransferDstOptimal, region);

  transition_image_layout(cmd_buffer, texture_image,
                          vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
                          vk::AccessFlagBits2::eTransferWrite, vk::AccessFlagBits2::eShaderRead,
                          vk::PipelineStageFlagBits2::eTransfer, vk::PipelineStageFlagBits2::eFragmentShader);

  end_single_time_commands(cmd_buffer);

  return bird::ok();
}

Result<void> VulkanRenderer::create_texture_image_view() {
  vk::ImageViewCreateInfo view_info{
      .image            = *texture_image, // Dereference to get the raw vk::Image
      .viewType         = vk::ImageViewType::e2D,
      .format           = vk::Format::eR8G8B8A8Srgb,
      .components       = {
          vk::ComponentSwizzle::eIdentity,
          vk::ComponentSwizzle::eIdentity,
          vk::ComponentSwizzle::eIdentity,
          vk::ComponentSwizzle::eIdentity
      },
      .subresourceRange = {
          .aspectMask     = vk::ImageAspectFlagBits::eColor,
          .baseMipLevel   = 0,
          .levelCount     = 1,
          .baseArrayLayer = 0,
          .layerCount     = 1
      }
  };

  auto vkr_create_view = logical_device.createImageView(view_info);
  if (vkr_create_view.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create texture image view");
  }

  texture_image_view = std::move(vkr_create_view.value);
  return bird::ok();
}

Result<void> VulkanRenderer::create_texture_sampler() {
  vk::SamplerCreateInfo sampler_info{
      .magFilter               = vk::Filter::eLinear, // Bilinear filtering when zoomed in
      .minFilter               = vk::Filter::eLinear, // Bilinear filtering when zoomed out
      .mipmapMode              = vk::SamplerMipmapMode::eLinear,
      .addressModeU            = vk::SamplerAddressMode::eRepeat,
      .addressModeV            = vk::SamplerAddressMode::eRepeat,
      .addressModeW            = vk::SamplerAddressMode::eRepeat,
      .mipLodBias              = 0.0f,
      .anisotropyEnable        = vk::False, // Disabled for now to avoid needing physical device feature checks
      .maxAnisotropy           = 1.0f,
      .compareEnable           = vk::False,
      .compareOp               = vk::CompareOp::eAlways,
      .minLod                  = 0.0f,
      .maxLod                  = 0.0f,
      .borderColor             = vk::BorderColor::eIntOpaqueBlack,
      .unnormalizedCoordinates = vk::False
  };

  auto vkr_create_sampler = logical_device.createSampler(sampler_info);
  if (vkr_create_sampler.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create texture sampler");
  }

  texture_sampler = std::move(vkr_create_sampler.value);
  return bird::ok();
}

Result<void> VulkanRenderer::create_descriptor_set_layout() {
  vk::DescriptorSetLayoutBinding ubo_binding{
      .binding            = 0,
      .descriptorType     = vk::DescriptorType::eUniformBuffer,
      .descriptorCount    = 1,
      .stageFlags         = vk::ShaderStageFlagBits::eVertex,
      .pImmutableSamplers = nullptr
  };

  vk::DescriptorSetLayoutBinding sampler_binding{
      .binding            = 1,
      .descriptorType     = vk::DescriptorType::eCombinedImageSampler,
      .descriptorCount    = 1,
      .stageFlags         = vk::ShaderStageFlagBits::eFragment,
      .pImmutableSamplers = nullptr
  };

  std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {ubo_binding, sampler_binding};

  vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_info{
      .bindingCount = static_cast<uint32_t>(bindings.size()),
      .pBindings    = bindings.data()
  };

  auto vkr_create_desc_set_layout = logical_device.createDescriptorSetLayout(descriptor_set_layout_info);
  if (vkr_create_desc_set_layout.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create descriptor set layout");
  }

  descriptor_set_layout = std::move(vkr_create_desc_set_layout.value);
  return bird::ok();
}

Result<void> VulkanRenderer::create_graphics_pipeline() {
  // loading the shaders from files
  auto r_load_vert_shader_module = load_shader_module<VertexShaderAsset>("core://shaders/vertices.vert.spv");
  if (!r_load_vert_shader_module) return bird::fail(r_load_vert_shader_module.error());

  auto r_load_frag_shader_module = load_shader_module<FragmentShaderAsset>("core://shaders/textures.frag.spv");
  if (!r_load_frag_shader_module) return bird::fail(r_load_frag_shader_module.error());

  std::unique_ptr<vk::raii::ShaderModule> vert_shader_module = std::move(r_load_vert_shader_module).value();
  std::unique_ptr<vk::raii::ShaderModule> frag_shader_module = std::move(r_load_frag_shader_module).value();

  // vertex shader stage create info
  vk::PipelineShaderStageCreateInfo vert_shader_stage_info{
    .stage = vk::ShaderStageFlagBits::eVertex,
    .module = **vert_shader_module,
    .pName = "main"
  };

  // fragment shader stage create info
  vk::PipelineShaderStageCreateInfo frag_shader_stage_info{
    .stage = vk::ShaderStageFlagBits::eFragment,
    .module = **frag_shader_module,
    .pName = "main"
  };

  vk::PipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

  std::vector<vk::DynamicState> dynamic_states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

  // pipeline layout create info
  vk::PipelineDynamicStateCreateInfo dynamic_state{
    .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
    .pDynamicStates = dynamic_states.data()
  };

  // vertex input create info
  auto binding_description    = Vertex::get_binding_descriptions();
  auto attribute_descriptions = Vertex::get_attr_descriptions();
  vk::PipelineVertexInputStateCreateInfo   vertex_input_info{
    .vertexBindingDescriptionCount   = 1,
      .pVertexBindingDescriptions      = &binding_description,
      .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size()),
      .pVertexAttributeDescriptions    = attribute_descriptions.data()
  };

  // input assembly create info
  vk::PipelineInputAssemblyStateCreateInfo input_assembly{.topology = vk::PrimitiveTopology::eTriangleList};

  // viewport and scissor
  vk::Viewport viewport{0.0f, static_cast<float>(swap_chain_extent.height), static_cast<float>(swap_chain_extent.width), -static_cast<float>(swap_chain_extent.height), 0.0f, 1.0f};
  vk::Rect2D scissor{vk::Offset2D{ 0, 0 }, swap_chain_extent};

  // viewport create info
  vk::PipelineViewportStateCreateInfo viewport_state{
    .viewportCount = 1,
    .pViewports = &viewport,
    .scissorCount = 1,
    .pScissors = &scissor
  };

  // rasterizer create info
  vk::PipelineRasterizationStateCreateInfo rasterizer{
    .depthClampEnable        = vk::False,
      .rasterizerDiscardEnable = vk::False,
      .polygonMode             = vk::PolygonMode::eFill,
      .cullMode                = vk::CullModeFlagBits::eBack,
      .frontFace               = vk::FrontFace::eCounterClockwise,
      .depthBiasEnable         = vk::False,
      .lineWidth               = 1.0f
  };

  // multisampling create info
  vk::PipelineMultisampleStateCreateInfo multisampling{
    .rasterizationSamples = vk::SampleCountFlagBits::e1,
    .sampleShadingEnable = vk::False
  };

  // disabling color blending for our only framebuffer
  vk::PipelineColorBlendAttachmentState color_blend_attachment{
      .blendEnable    = vk::False,
      .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
  };

  vk::PipelineColorBlendStateCreateInfo color_blending{
      .logicOpEnable = vk::False,
      .logicOp = vk::LogicOp::eCopy,
      .attachmentCount = 1,
      .pAttachments = &color_blend_attachment
  };

  // pipeline layout create info
  vk::PipelineLayoutCreateInfo pipeline_layout_info{
      .setLayoutCount = 1,
      .pSetLayouts    = &*descriptor_set_layout
  };

  // creating the pipeline layout
  auto vkr_create_pipeline_layout = logical_device.createPipelineLayout(pipeline_layout_info);
  if (vkr_create_pipeline_layout.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create pipeline layout");
  }

  pipeline_layout = std::move(vkr_create_pipeline_layout.value);

  vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipeline_create_info_chain = {
      {
          .stageCount          = 2,
          .pStages             = shader_stages,
          .pVertexInputState   = &vertex_input_info,
          .pInputAssemblyState = &input_assembly,
          .pViewportState      = &viewport_state,
          .pRasterizationState = &rasterizer,
          .pMultisampleState   = &multisampling,
          .pColorBlendState    = &color_blending,
          .pDynamicState       = &dynamic_state,
          .layout              = pipeline_layout,
          .renderPass          = nullptr
      },
      {
          .colorAttachmentCount    = 1,
          .pColorAttachmentFormats = &swap_chain_surface_format.format
      }
  };


  auto vkr_create_graphics_pipeline = logical_device.createGraphicsPipeline(nullptr, pipeline_create_info_chain.get<vk::GraphicsPipelineCreateInfo>());
  if (vkr_create_graphics_pipeline.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create graphics pipeline");
  }
  graphics_pipeline = std::move(vkr_create_graphics_pipeline.value);

  return bird::ok();
}

Result<void> VulkanRenderer::create_command_pool() {
  vk::CommandPoolCreateInfo pool_info{
    .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
    .queueFamilyIndex = queue_index
  };

  auto vkr_create_command_pool = logical_device.createCommandPool(pool_info);
  if (vkr_create_command_pool.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create command pool");
  }

  command_pool = std::move(vkr_create_command_pool.value);

  return bird::ok();
}

Result<void> VulkanRenderer::create_vertex_buffer() {
  return create_buffer(vk::BufferUsageFlagBits::eVertexBuffer, vertices, vertex_buffer);
}

Result<void> VulkanRenderer::create_index_buffer() {
  return create_buffer(vk::BufferUsageFlagBits::eIndexBuffer, indices, index_buffer);;
}

Result<void> VulkanRenderer::create_uniform_buffers() {
  vk::DeviceSize buffer_size = sizeof(ViewData);

  vk::BufferCreateInfo buffer_info{
      .size        = buffer_size,
      .usage       = vk::BufferUsageFlagBits::eUniformBuffer,
      .sharingMode = vk::SharingMode::eExclusive
  };

  vma::AllocationCreateInfo alloc_info{
      // eMapped maps the memory immediately upon creation
      // eHostAccessSequentialWrite is optimal for uniform buffers updated frame-by-frame
      .flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite |
               vma::AllocationCreateFlagBits::eMapped,
      .usage = vma::MemoryUsage::eAuto
  };

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    auto vkr_create_buffer = vma_allocator.createBuffer(buffer_info, alloc_info);

    if (vkr_create_buffer.result != vk::Result::eSuccess) {
      return bird::fail("Failed to create uniform buffer via VMA-Hpp");
    }

    uniform_buffers.push_back(std::move(vkr_create_buffer.value));

    // Retrieve the persistently mapped pointer directly from the VMA allocation info
    uniform_buffers_mapped[i] = uniform_buffers[i].getAllocation().getInfo().pMappedData;
  }

  return bird::ok();
}

Result<void> VulkanRenderer::create_descriptor_pool() {
  std::array<vk::DescriptorPoolSize, 2> pool_sizes = {{
    {
      .type            = vk::DescriptorType::eUniformBuffer,
      .descriptorCount = MAX_FRAMES_IN_FLIGHT
          },
          {
      .type            = vk::DescriptorType::eCombinedImageSampler,
      .descriptorCount = MAX_FRAMES_IN_FLIGHT
          }
  }};

  vk::DescriptorPoolCreateInfo pool_info{
      .flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets       = MAX_FRAMES_IN_FLIGHT,
      .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
      .pPoolSizes    = pool_sizes.data()
  };

  auto vkr_create = logical_device.createDescriptorPool(pool_info);
  if (vkr_create.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create descriptor pool");
  }
  descriptor_pool = std::move(vkr_create.value);

  std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptor_set_layout);
  vk::DescriptorSetAllocateInfo alloc_info{
      .descriptorPool     = *descriptor_pool,
      .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
      .pSetLayouts        = layouts.data()
  };

  auto vkr_alloc_sets = logical_device.allocateDescriptorSets(alloc_info);
  if (vkr_alloc_sets.result != vk::Result::eSuccess) {
    return bird::fail("Failed to allocate descriptor sets");
  }
  descriptor_sets = std::move(vkr_alloc_sets.value);

  // Write the buffers and images to the descriptors
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DescriptorBufferInfo buffer_info{
        .buffer = *uniform_buffers[i],
        .offset = 0,
        .range  = sizeof(ViewData)
    };

    vk::DescriptorImageInfo image_info{
        .sampler     = *texture_sampler,
        .imageView   = *texture_image_view,
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal // Must match layout transitioned to in Step 2!
    };

    std::array<vk::WriteDescriptorSet, 2> descriptor_writes = {{
      {
        .dstSet          = *descriptor_sets[i],
        .dstBinding      = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType  = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo     = &buffer_info
            },
            {
        .dstSet          = *descriptor_sets[i],
        .dstBinding      = 1,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType  = vk::DescriptorType::eCombinedImageSampler,
        .pImageInfo      = &image_info
            }
    }};

    // Because vulkan.hpp accepts ArrayProxy, we can pass the std::array directly
    logical_device.updateDescriptorSets(descriptor_writes, nullptr);
  }

  return bird::ok();
}

Result<void> VulkanRenderer::create_command_buffers() {
  vk::CommandBufferAllocateInfo alloc_info{
    .commandPool = command_pool,
    .level = vk::CommandBufferLevel::ePrimary,
    .commandBufferCount = MAX_FRAMES_IN_FLIGHT
  };

  auto vkr_alloc_command_buffers = logical_device.allocateCommandBuffers(alloc_info);
  if (vkr_alloc_command_buffers.result != vk::Result::eSuccess) {
    return bird::fail("Failed to allocate command buffers");
  }
  command_buffers = std::move(vkr_alloc_command_buffers.value);

  return bird::ok();
}

Result<void> VulkanRenderer::create_sync_objects() {
  present_complete_semaphores.clear();
  draw_fences.clear();
  render_finished_semaphores.clear();

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    auto vkr_present_complete = logical_device.createSemaphore(vk::SemaphoreCreateInfo());
    if (vkr_present_complete.result != vk::Result::eSuccess) {
      return bird::fail("Failed to create present complete semaphore");
    }
    present_complete_semaphores.push_back(std::move(vkr_present_complete.value));

    auto vkr_draw_fence = logical_device.createFence(
        vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
    if (vkr_draw_fence.result != vk::Result::eSuccess) {
      return bird::fail("Failed to create draw fence");
    }
    draw_fences.push_back(std::move(vkr_draw_fence.value));
  }

  // One render finished semaphore per swapchain image
  for (size_t i = 0; i < swap_chain_images.size(); i++) {
    auto vkr_render_finished = logical_device.createSemaphore(vk::SemaphoreCreateInfo());
    if (vkr_render_finished.result != vk::Result::eSuccess) {
      return bird::fail("Failed to create render finished semaphore");
    }
    render_finished_semaphores.push_back(std::move(vkr_render_finished.value));
  }

  return bird::ok();
}

Result<void> VulkanRenderer::record_command_buffer() {
  auto vkr_command_buffer_begin = command_buffers[current_frame_].begin({});
  if (vkr_command_buffer_begin != vk::Result::eSuccess) {
    return bird::fail("Failed to begin command buffer recording");
  }

  // Transition the image layout for rendering
  transition_image_layout(
      command_buffers[current_frame_],
      swap_chain_images[current_image_index_],
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eColorAttachmentOptimal,
      {},
      vk::AccessFlagBits2::eColorAttachmentWrite,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput
  );

  // Set up the color attachment
  vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 1.0f, 0.5f, 1.0f);
  vk::RenderingAttachmentInfo attachmentInfo = {
      .imageView   = swap_chain_image_views[current_image_index_],
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp      = vk::AttachmentLoadOp::eClear,
      .storeOp     = vk::AttachmentStoreOp::eStore,
      .clearValue  = clearColor};

  // Set up the rendering info
  vk::RenderingInfo renderingInfo = {
      .renderArea           = {.offset = {0, 0}, .extent = swap_chain_extent},
      .layerCount           = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments    = &attachmentInfo};

  // Begin rendering
  command_buffers[current_frame_].beginRendering(renderingInfo);

  command_buffers[current_frame_].bindPipeline(vk::PipelineBindPoint::eGraphics, *graphics_pipeline);
  command_buffers[current_frame_].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipeline_layout, 0, {*descriptor_sets[current_frame_]},nullptr);
  command_buffers[current_frame_].bindVertexBuffers(0, *vertex_buffer, {0});
  command_buffers[current_frame_].bindIndexBuffer(index_buffer, 0, vk::IndexType::eUint16);
  command_buffers[current_frame_].setViewport(0, vk::Viewport(0.0f, static_cast<float>(swap_chain_extent.height), static_cast<float>(swap_chain_extent.width), -static_cast<float>(swap_chain_extent.height), 0.0f, 1.0f));
  command_buffers[current_frame_].setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swap_chain_extent));
  command_buffers[current_frame_].drawIndexed(indices.size(), 1, 0, 0, 0);

  // End rendering
  command_buffers[current_frame_].endRendering();

  // Transition the image layout for presentation
  transition_image_layout(
      command_buffers[current_frame_],
      swap_chain_images[current_image_index_],
      vk::ImageLayout::eColorAttachmentOptimal,
      vk::ImageLayout::ePresentSrcKHR,
      vk::AccessFlagBits2::eColorAttachmentWrite,
      {},
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      vk::PipelineStageFlagBits2::eBottomOfPipe
  );

  auto vkr_command_buffer_end = command_buffers[current_frame_].end();
  if (vkr_command_buffer_end != vk::Result::eSuccess) {
    return bird::fail("Failed to end command buffer recording");
  }

  return bird::ok();
}

Result<vk::raii::CommandBuffer> VulkanRenderer::begin_single_time_commands() {
  vk::CommandBufferAllocateInfo alloc_info{
      .commandPool        = *command_pool,
      .level              = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = 1
  };

  auto vkr_alloc = logical_device.allocateCommandBuffers(alloc_info);
  if (vkr_alloc.result != vk::Result::eSuccess) {
    return bird::fail("Failed to allocate single time command buffer");
  }

  vk::raii::CommandBuffer command_buffer = std::move(vkr_alloc.value[0]);

  vk::CommandBufferBeginInfo begin_info{
      .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
  };

  if (command_buffer.begin(begin_info) != vk::Result::eSuccess) {
    return bird::fail("Failed to begin single time command buffer");
  }

  return std::move(command_buffer);
}

void VulkanRenderer::end_single_time_commands(vk::raii::CommandBuffer& command_buffer) {
  auto vkr_end = command_buffer.end();
  // (Ignoring result check for brevity, could optionally return Result<void>)

  vk::SubmitInfo submit_info{
      .commandBufferCount = 1,
      .pCommandBuffers    = &*command_buffer
  };

  graphics_queue.submit(submit_info, nullptr);
  graphics_queue.waitIdle();
}

void VulkanRenderer::transition_image_layout(
    vk::raii::CommandBuffer& cmd_buffer,
    vk::Image                image,
    vk::ImageLayout         old_layout,
    vk::ImageLayout         new_layout,
    vk::AccessFlags2        src_access_mask,
    vk::AccessFlags2        dst_access_mask,
    vk::PipelineStageFlags2 src_stage_mask,
    vk::PipelineStageFlags2 dst_stage_mask)
{
  vk::ImageMemoryBarrier2 barrier =
      {
      .srcStageMask        = src_stage_mask,
      .srcAccessMask       = src_access_mask,
      .dstStageMask        = dst_stage_mask,
      .dstAccessMask       = dst_access_mask,
      .oldLayout           = old_layout,
      .newLayout           = new_layout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image               = image,
      .subresourceRange    = {
          .aspectMask     = vk::ImageAspectFlagBits::eColor,
          .baseMipLevel   = 0,
          .levelCount     = 1,
          .baseArrayLayer = 0,
          .layerCount     = 1}
      };

  vk::DependencyInfo dependency_info = {
      .dependencyFlags         = {},
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers    = &barrier
  };

  cmd_buffer.pipelineBarrier2(dependency_info);
}

void VulkanRenderer::begin_frame() {
  if (needs_framebuffer_resize_ || is_context_lost_) {
    return;
  }

  auto vkr_fence = logical_device.waitForFences(*draw_fences[current_frame_], vk::True, UINT64_MAX);
  if (vkr_fence == vk::Result::eErrorDeviceLost) {
    is_context_lost_ = true;
    return;
  } else if (vkr_fence != vk::Result::eSuccess) {
    std::cerr << "Fatal: Failed to wait for fence. Result: " << vk::to_string(vkr_fence) << std::endl;
    return;
  }

  logical_device.resetFences(*draw_fences[current_frame_]);
}

void VulkanRenderer::begin_pass(const ViewData& view_data) {
  if (needs_framebuffer_resize_ || is_context_lost_) {
    return;
  }

  update_ubo(view_data);

  VkResult vkr_acquire_next_image = vkAcquireNextImageKHR(
      *logical_device,
      *swap_chain,
      UINT64_MAX,
      *present_complete_semaphores[current_frame_],
      VK_NULL_HANDLE,
      &current_image_index_
  );

  if (vkr_acquire_next_image == VK_ERROR_OUT_OF_DATE_KHR || vkr_acquire_next_image == VK_SUBOPTIMAL_KHR) {
    needs_framebuffer_resize_ = true;
    return;
  } else if (vkr_acquire_next_image == VK_ERROR_DEVICE_LOST) {
    is_context_lost_ = true;
    return;
  } else if (vkr_acquire_next_image != VK_SUCCESS) {
    std::cerr << "Fatal: Failed to acquire swap chain image. Result: " << vkr_acquire_next_image << std::endl;
    return;
  }
}

void VulkanRenderer::end_pass() {
  if (needs_framebuffer_resize_ || is_context_lost_) {
    return;
  }

  (void) record_command_buffer();
}

void VulkanRenderer::end_frame() {
  if (needs_framebuffer_resize_ || is_context_lost_) {
    return;
  }

  vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
  const vk::SubmitInfo   submitInfo{
      .waitSemaphoreCount   = 1,
      .pWaitSemaphores      = &*present_complete_semaphores[current_frame_],
      .pWaitDstStageMask    = &waitDestinationStageMask,
      .commandBufferCount   = 1,
      .pCommandBuffers      = &*command_buffers[current_frame_],
      .signalSemaphoreCount = 1,
      .pSignalSemaphores    = &*render_finished_semaphores[current_image_index_]
  };

  graphics_queue.submit(submitInfo, *draw_fences[current_frame_]);

  const vk::PresentInfoKHR presentInfoKHR{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores    = &*render_finished_semaphores[current_image_index_],
      .swapchainCount     = 1,
      .pSwapchains        = &*swap_chain,
      .pImageIndices      = &current_image_index_,
  };

  VkResult vkr_present = vkQueuePresentKHR(
      *graphics_queue,
      reinterpret_cast<const VkPresentInfoKHR*>(&presentInfoKHR)
  );

  if (vkr_present == VK_ERROR_OUT_OF_DATE_KHR || vkr_present == VK_SUBOPTIMAL_KHR) {
    needs_framebuffer_resize_ = true;
  } else if (vkr_present == VK_ERROR_DEVICE_LOST) {
    is_context_lost_ = true;
  } else if (vkr_present != VK_SUCCESS) {
    std::cerr << "Fatal: Failed to present image. Result: " << vkr_present << "\n";
  }

  move_to_next_frame();
}

inline void VulkanRenderer::move_to_next_frame() {
  current_frame_++;
  if (current_frame_ >= MAX_FRAMES_IN_FLIGHT) {
    current_frame_ = 0;
  }
}

void VulkanRenderer::update_ubo(const ViewData& view_data) {
  memcpy(uniform_buffers_mapped[current_frame_], &view_data, sizeof(view_data));
}

bool VulkanRenderer::is_context_lost() const noexcept {
  return is_context_lost_;
}

bool VulkanRenderer::needs_framebuffer_resize() const noexcept {
  return needs_framebuffer_resize_;
}

Result<void> VulkanRenderer::resize_framebuffer(uint32_t width, uint32_t height) {
  window_width = width;
  window_height = height;

  auto r_recreate_swap_chain = recreate_swap_chain();
  if (!r_recreate_swap_chain) return r_recreate_swap_chain;

  needs_framebuffer_resize_ = false;

  return bird::ok();
}

// TODO
//
void VulkanRenderer::submit(const RenderCommand render_command) {}
//
} // bird