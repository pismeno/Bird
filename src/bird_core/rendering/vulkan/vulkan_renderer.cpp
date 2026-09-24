#include "rendering/vulkan/vulkan_renderer.hpp"

#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <string>
#include <memory>
#include <filesystem>

#include <resources/asset_handle.hpp>
#include <resources/asset_manager.hpp>
#include <resources/asset_types.hpp>
#include <utils/result.hpp>

namespace bird {

Result<void> VulkanRenderer::attach_window(void* native_window_handle, uint32_t window_width, uint32_t window_height) {
  this->native_window_handle = native_window_handle;
  this->window_width = window_width;
  this->window_height = window_height;
  return bird::ok();
}

Result<void> VulkanRenderer::init() {
  auto r_create_instance = create_instance();
  if (!r_create_instance) return r_create_instance;

  std::cout << "Vulkan Instance created successfully" << std::endl;

  auto r_pick_physical_device = pick_physical_device();
  if (!r_pick_physical_device) return r_pick_physical_device;

  std::cout << "Vulkan Physical Device selected successfully" << std::endl;

  auto r_create_logical_device = create_logical_device();
  if (!r_create_logical_device) return r_create_logical_device;

  std::cout << "Vulkan Logical Device created successfully" << std::endl;

  auto r_create_swap_chain = create_swap_chain();
  if (!r_create_swap_chain) return r_create_swap_chain;

  std::cout << "Vulkan Swap Chain created successfully" << std::endl;

  auto r_create_image_views = create_image_views();
  if (!r_create_image_views) return r_create_image_views;

  std::cout << "Vulkan Image Views created successfully" << std::endl;

  auto r_create_graphics_pipeline = create_graphics_pipeline();
  if (!r_create_graphics_pipeline) return r_create_graphics_pipeline;

  std::cout << "Vulkan Graphics Pipeline created successfully" << std::endl;

  auto r_create_command_pool = create_command_pool();
  if (!r_create_command_pool) return r_create_command_pool;

  std::cout << "Vulkan Command Pool created successfully" << std::endl;

  auto r_create_command_buffer = create_command_buffer();
  if (!r_create_command_buffer) return r_create_command_buffer;

  std::cout << "Vulkan Command Buffer created successfully" << std::endl;

  auto r_create_sync_objects = create_sync_objects();
  if (!r_create_sync_objects) return r_create_sync_objects;

  std::cout << "Vulkan Sync Objects created successfully" << std::endl;

  std::cout << "Vulkan Renderer initialized successfully" << std::endl;

  return bird::ok();
}

Result<void> VulkanRenderer::create_instance() {
  vk::ApplicationInfo app_info{
      .pApplicationName = "Hello Triangle",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "BirdEngine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_3
  };

  std::vector<const char*> extensions = { "VK_KHR_surface" };
#ifdef BIRD_PLATFORM_WINDOWS
  extensions.push_back("VK_KHR_win32_surface");
#endif
#ifdef BIRD_PLATFORM_LINUX
  extensions.push_back("VK_KHR_xcb_surface");
#endif

  vk::InstanceCreateInfo create_info{
      .pApplicationInfo = &app_info,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data()
  };

  // STEP 1: Create raw handle, check result
  auto inst_res = vk::createInstance(create_info);
  if (inst_res.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create Vulkan Instance");
  }

  // STEP 2: Transfer ownership to RAII wrapper
  instance = vk::raii::Instance(context, inst_res.value);

  // We must create the surface immediately after the instance
  return create_surface();
}

Result<void> VulkanRenderer::create_surface() {
  vk::Win32SurfaceCreateInfoKHR createInfo{
      .hinstance = GetModuleHandle(nullptr),
      .hwnd      = static_cast<HWND>(native_window_handle)
  };

  // Dereference RAII object to get the raw vk::Instance wrapper to call creation
  vk::Instance raw_instance = *instance;
  auto surf_res = raw_instance.createWin32SurfaceKHR(createInfo);
  if (surf_res.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create Win32 Surface");
  }

  surface = vk::raii::SurfaceKHR(instance, surf_res.value);
  return bird::ok();
}

Result<void> VulkanRenderer::pick_physical_device() {
  auto phys_devs_res = instance.enumeratePhysicalDevices();
  if (phys_devs_res.result != vk::Result::eSuccess) {
    return bird::fail("Failed to enumerate physical devices");
  }

  std::vector<vk::raii::PhysicalDevice> physicalDevices = phys_devs_res.value;
  auto const devIter = std::ranges::find_if(physicalDevices, [&]( auto const & physicalDevice) {
    return is_physical_device_suitable(physicalDevice);
  });

  if (devIter == physicalDevices.end()) {
    return bird::fail("No suitable physical device found");
  }

  physical_device = *devIter;
  return bird::ok();
}

bool VulkanRenderer::is_physical_device_suitable(const vk::PhysicalDevice &physical_device) const {
  std::vector<const char*> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};

  bool supportsVulkan1_3 = physical_device.getProperties().apiVersion >= vk::ApiVersion13;

  // getQueueFamilyProperties doesn't fail, so it returns the vector directly (no ResultValue)
  auto queueFamilies    = physical_device.getQueueFamilyProperties();
  bool supportsGraphics = std::ranges::any_of( queueFamilies, []( auto const & qfp ) { return !!( qfp.queueFlags & vk::QueueFlagBits::eGraphics ); } );

  auto ext_res = physical_device.enumerateDeviceExtensionProperties();
  if (ext_res.result != vk::Result::eSuccess) return false;
  auto availableDeviceExtensions = ext_res.value;

  bool supportsAllRequiredExtensions =
      std::ranges::all_of( requiredDeviceExtension,
                           [&availableDeviceExtensions]( auto const & requiredDeviceExtension )
                           {
                             return std::ranges::any_of( availableDeviceExtensions,
                                                         [requiredDeviceExtension]( auto const & availableDeviceExtension )
                                                         { return strcmp( availableDeviceExtension.extensionName, requiredDeviceExtension ) == 0; } );
                           } );

  auto features                 = physical_device.template getFeatures2<vk::PhysicalDeviceFeatures2,
      vk::PhysicalDeviceVulkan11Features,
      vk::PhysicalDeviceVulkan13Features,
      vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
  bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                                  features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                                  features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

  return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
}

Result<void> VulkanRenderer::create_logical_device() {
  std::vector<vk::QueueFamilyProperties> queue_family_properties = physical_device.getQueueFamilyProperties();

  // get the first index into queue_family_properties which supports both graphics and present
  for (uint32_t qfp_index = 0; qfp_index < queue_family_properties.size(); qfp_index++) {

    // FIX 1: Extract the ResultValue properly instead of treating it as a raw bool
    auto support_res = physical_device.getSurfaceSupportKHR(qfp_index, *surface);
    bool supports_present = (support_res.result == vk::Result::eSuccess) && support_res.value;

    if ((queue_family_properties[qfp_index].queueFlags & vk::QueueFlagBits::eGraphics) && supports_present) {
      // found a queue family that supports both graphics and present
      queue_index = qfp_index;
      break;
    }
  }

  if (queue_index == ~0u) {
    return bird::fail("Could not find a queue for graphics and present");
  }

  // query for Vulkan 1.3 features
  vk::StructureChain<vk::PhysicalDeviceFeatures2,
      vk::PhysicalDeviceVulkan11Features,
      vk::PhysicalDeviceVulkan13Features,
      vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
      feature_chain = {
      {},                                                          // vk::PhysicalDeviceFeatures2
      {.shaderDrawParameters = true},                              // vk::PhysicalDeviceVulkan11Features
      {.synchronization2 = true, .dynamicRendering = true},        // vk::PhysicalDeviceVulkan13Features
      {.extendedDynamicState = true}                               // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
  };

  // create a Device
  float queue_priority = 0.5f;

  vk::DeviceQueueCreateInfo device_queue_create_info{
      .queueFamilyIndex = queue_index,
      .queueCount       = 1,
      .pQueuePriorities = &queue_priority
  };

  // FIX 2: Declare the required extensions locally before using them
  std::vector<const char*> required_device_extensions = {vk::KHRSwapchainExtensionName};

  vk::DeviceCreateInfo device_create_info{
      .pNext                   = &feature_chain.get<vk::PhysicalDeviceFeatures2>(),
      .queueCreateInfoCount    = 1,
      .pQueueCreateInfos       = &device_queue_create_info,
      .enabledExtensionCount   = static_cast<uint32_t>(required_device_extensions.size()),
      .ppEnabledExtensionNames = required_device_extensions.data()
  };

  auto vkr_create_logical_device = physical_device.createDevice(device_create_info);
  if (vkr_create_logical_device.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create logical device");
  }
  logical_device = std::move(vkr_create_logical_device.value);

  graphics_queue = logical_device.getQueue(queue_index, 0);

  return bird::ok();
}

Result<void> VulkanRenderer::create_swap_chain() {
  auto caps_res = physical_device.getSurfaceCapabilitiesKHR(*surface);
  if (caps_res.result != vk::Result::eSuccess) return bird::fail("Failed to get surface capabilities");
  vk::SurfaceCapabilitiesKHR surface_capabilities = caps_res.value;

  auto formats_res = physical_device.getSurfaceFormatsKHR(*surface);
  if (formats_res.result != vk::Result::eSuccess) return bird::fail("Failed to get surface formats");
  std::vector<vk::SurfaceFormatKHR> available_formats = formats_res.value;

  auto modes_res = physical_device.getSurfacePresentModesKHR(*surface);
  if (modes_res.result != vk::Result::eSuccess) return bird::fail("Failed to get present modes");
  std::vector<vk::PresentModeKHR> availablePresentModes = modes_res.value;

  swap_chain_extent = choose_swap_extent(surface_capabilities);
  uint32_t minImageCount = choose_swap_min_image_count(surface_capabilities);
  swap_chain_surface_format = choose_swap_surface_format(available_formats);

  vk::SwapchainCreateInfoKHR swap_chain_create_info{
      .surface          = *surface,
      .minImageCount    = minImageCount,
      .imageFormat      = swap_chain_surface_format.format,
      .imageColorSpace  = swap_chain_surface_format.colorSpace,
      .imageExtent      = swap_chain_extent,
      .imageArrayLayers = 1,
      .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
      .imageSharingMode = vk::SharingMode::eExclusive,
      .preTransform     = surface_capabilities.currentTransform,
      .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode      = choose_swap_present_mode(availablePresentModes),
      .clipped          = true
  };

  vk::Device raw_dev = *logical_device;
  auto swap_res = raw_dev.createSwapchainKHR(swap_chain_create_info);
  if (swap_res.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create swapchain");
  }
  swap_chain = vk::raii::SwapchainKHR(logical_device, swap_res.value);

  auto images_res = swap_chain.getImages();
  if (images_res.result != vk::Result::eSuccess) {
    return bird::fail("Failed to retrieve swapchain images");
  }
  swap_chain_images = images_res.value;

  return bird::ok();
}

vk::SurfaceFormatKHR VulkanRenderer::choose_swap_surface_format(std::vector<vk::SurfaceFormatKHR> const &availableFormats) {
  const auto formatIt = std::ranges::find_if(
      availableFormats,
      [](const auto &format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
  return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

vk::PresentModeKHR VulkanRenderer::choose_swap_present_mode(std::vector<vk::PresentModeKHR> const &availablePresentModes)
{
  assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) {return presentMode == vk::PresentModeKHR::eFifo;}));
  return std::ranges::any_of(availablePresentModes,
                             [](const vk::PresentModeKHR value) {return vk::PresentModeKHR::eMailbox == value;}) ?
         vk::PresentModeKHR::eMailbox :
         vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanRenderer::choose_swap_extent(vk::SurfaceCapabilitiesKHR const &capabilities)
{
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
  {
    return capabilities.currentExtent;
  }

  return {
      std::clamp<uint32_t>(window_width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
      std::clamp<uint32_t>(window_height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
  };
}

uint32_t VulkanRenderer::choose_swap_min_image_count(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities)
{
  auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
  if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
  {
    minImageCount = surfaceCapabilities.maxImageCount;
  }
  return minImageCount;
}

Result<void> VulkanRenderer::create_image_views() {
  assert(swap_chain_image_views.empty());

  vk::ImageViewCreateInfo image_view_create_info{
    .viewType         = vk::ImageViewType::e2D,
    .format           = swap_chain_surface_format.format,
    .components       = {vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity},
    .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1},
  };

  vk::Device raw_device = *logical_device;
  for (auto &image : swap_chain_images)
  {
    image_view_create_info.image = image;

    auto view_res = raw_device.createImageView(image_view_create_info);
    if (view_res.result != vk::Result::eSuccess) {
      return bird::fail("Failed to create swapchain image view");
    }

    swap_chain_image_views.emplace_back(logical_device, view_res.value);
  }

  return bird::ok();
}

Result<void> VulkanRenderer::create_graphics_pipeline() {
  // loading the shaders from files
  auto r_load_vert_shader_module = load_shader_module<VertexShaderAsset>("shaders/triangle.vert.glsl");
  if (!r_load_vert_shader_module) return bird::fail(r_load_vert_shader_module.error());

  auto r_load_frag_shader_module = load_shader_module<FragmentShaderAsset>("shaders/triangle.frag.glsl");
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
  vk::PipelineVertexInputStateCreateInfo vertex_input_info;

  // input assembly create info
  vk::PipelineInputAssemblyStateCreateInfo input_assembly{.topology = vk::PrimitiveTopology::eTriangleList};

  // viewport and scissor
  vk::Viewport viewport{0.0f, 0.0f, static_cast<float>(swap_chain_extent.width), static_cast<float>(swap_chain_extent.height), 0.0f, 1.0f};
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
      .frontFace               = vk::FrontFace::eClockwise,
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
    .setLayoutCount = 0,
    .pushConstantRangeCount = 0
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

Result<void> VulkanRenderer::create_command_buffer() {
  vk::CommandBufferAllocateInfo alloc_info{
    .commandPool = command_pool,
    .level = vk::CommandBufferLevel::ePrimary,
    .commandBufferCount = 1
  };

  auto vkr_alloc_command_buffers = logical_device.allocateCommandBuffers(alloc_info);
  if (vkr_alloc_command_buffers.result != vk::Result::eSuccess) {
    return bird::fail("Failed to allocate command buffers");
  }
  command_buffer = std::move(vkr_alloc_command_buffers.value.front());

  return bird::ok();
}

Result<void> VulkanRenderer::create_sync_objects() {
  auto vkr_create_present_complete_semaphore = logical_device.createSemaphore(vk::SemaphoreCreateInfo());
  if (vkr_create_present_complete_semaphore.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create present complete semaphore");
  }
  present_complete_semaphore = std::move(vkr_create_present_complete_semaphore.value);

  auto vkr_create_render_finished_semaphore = logical_device.createSemaphore(vk::SemaphoreCreateInfo());
  if (vkr_create_render_finished_semaphore.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create render finished semaphore");
  }
  render_finished_semaphore = std::move(vkr_create_render_finished_semaphore.value);

  auto vkr_create_draw_fence = logical_device.createFence(vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
  if (vkr_create_draw_fence.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create draw fence");
  }
  draw_fence = std::move(vkr_create_draw_fence.value);

  return bird::ok();
}

Result<void> VulkanRenderer::record_command_buffer(const uint32_t image_index) {
  auto vkr_command_buffer_begin = command_buffer.begin({});
  if (vkr_command_buffer_begin != vk::Result::eSuccess) {
    return bird::fail("Failed to begin command buffer recording");
  }

  // Transition the image layout for rendering
  transition_image_layout(
      image_index,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eColorAttachmentOptimal,
      {},
      vk::AccessFlagBits2::eColorAttachmentWrite,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput
  );

  // Set up the color attachment
  vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
  vk::RenderingAttachmentInfo attachmentInfo = {
      .imageView   = swap_chain_image_views[image_index],
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
  command_buffer.beginRendering(renderingInfo);

  command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphics_pipeline);
  command_buffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swap_chain_extent.width), static_cast<float>(swap_chain_extent.height), 0.0f, 1.0f));
  command_buffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swap_chain_extent));
  command_buffer.draw(3, 1, 0, 0);

  // End rendering
  command_buffer.endRendering();

  // Transition the image layout for presentation
  transition_image_layout(
      image_index,
      vk::ImageLayout::eColorAttachmentOptimal,
      vk::ImageLayout::ePresentSrcKHR,
      vk::AccessFlagBits2::eColorAttachmentWrite,
      {},
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      vk::PipelineStageFlagBits2::eBottomOfPipe
  );

  auto vkr_command_buffer_end = command_buffer.end();
  if (vkr_command_buffer_end != vk::Result::eSuccess) {
    return bird::fail("Failed to end command buffer recording");
  }

  return bird::ok();
}

void VulkanRenderer::transition_image_layout(
    uint32_t                imageIndex,
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
      .image               = swap_chain_images[imageIndex],
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

  command_buffer.pipelineBarrier2(dependency_info);
}

void VulkanRenderer::render() {
  if (needs_framebuffer_resize_ || is_context_lost_) {
    return;
  }

  auto vkr_fence = logical_device.waitForFences(*draw_fence, vk::True, UINT64_MAX);
  if (vkr_fence == vk::Result::eErrorDeviceLost) {
    is_context_lost_ = true;
    return;
  } else if (vkr_fence != vk::Result::eSuccess) {
    std::cerr << "Fatal: Failed to wait for fence. Result: " << vk::to_string(vkr_fence) << std::endl;
    return;
  }

  logical_device.resetFences(*draw_fence);

  uint32_t image_index = 0;
  VkResult vkr_acquire_next_image = vkAcquireNextImageKHR(
      *logical_device,
      *swap_chain,
      UINT64_MAX,
      *present_complete_semaphore,
      VK_NULL_HANDLE,
      &image_index
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

  auto _r = record_command_buffer(image_index);

  vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
  const vk::SubmitInfo   submitInfo{
      .waitSemaphoreCount   = 1,
      .pWaitSemaphores      = &*present_complete_semaphore,
      .pWaitDstStageMask    = &waitDestinationStageMask,
      .commandBufferCount   = 1,
      .pCommandBuffers      = &*command_buffer,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores    = &*render_finished_semaphore
  };

  graphics_queue.submit(submitInfo, *draw_fence);

  const vk::PresentInfoKHR presentInfoKHR{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores    = &*render_finished_semaphore,
      .swapchainCount     = 1,
      .pSwapchains        = &*swap_chain,
      .pImageIndices      = &image_index,
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
}

Result<void> VulkanRenderer::recreate_swap_chain() {
  logical_device.waitIdle();

  swap_chain_image_views.clear();
  swap_chain = nullptr;

  auto r_create_swap_chain = create_swap_chain(); // TODO later it should be done directly here to pass current swap chain into create swap chain struct as an old to not lose render commands
  if (!r_create_swap_chain) return r_create_swap_chain;

  auto r_create_image_views = create_image_views();
  if (!r_create_image_views) return r_create_image_views;

  return bird::ok();
}

Result<void> VulkanRenderer::shutdown() {
  if (!is_context_lost()) {
    logical_device.waitIdle();
  }

  return bird::ok();
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
void VulkanRenderer::submit_quad(const RenderCommand render_command) {}
//
} // namespace bird