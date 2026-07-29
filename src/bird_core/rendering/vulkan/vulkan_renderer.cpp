#include "rendering/vulkan/vulkan_renderer.hpp"

#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <string>
#include <memory>

#include "rendering/shader_compiler.hpp"
#include "utils/result.hpp"

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

  auto r_pick_physical_device = pick_physical_device();
  if (!r_pick_physical_device) return r_pick_physical_device;

  auto r_create_logical_device = create_logical_device();
  if (!r_create_logical_device) return r_create_logical_device;

  auto r_create_swap_chain = create_swap_chain();
  if (!r_create_swap_chain) return r_create_swap_chain;

  auto r_create_image_views = create_image_views();
  if (!r_create_image_views) return r_create_image_views;

  auto r_create_graphics_pipeline = create_graphics_pipeline();
  if (!r_create_graphics_pipeline) return r_create_graphics_pipeline;

  std::cout << "Vulkan Renderer initialized successfully" << std::endl;

  return bird::ok();
}

Result<void> VulkanRenderer::create_instance() {
  vk::ApplicationInfo app_info{
      .pApplicationName = "Hello Triangle",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "BirdEngine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_0
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
  auto queue_family_properties = physical_device.getQueueFamilyProperties();

  auto it = std::ranges::find_if(queue_family_properties, [this, index = 0u](const auto& qfp) mutable {
    uint32_t current_index = index++;
    bool supports_graphics = static_cast<bool>(qfp.queueFlags & vk::QueueFlagBits::eGraphics);

    auto support_res = physical_device.getSurfaceSupportKHR(current_index, *surface);
    bool supports_present = (support_res.result == vk::Result::eSuccess) && support_res.value;

    return supports_graphics && supports_present;
  });

  if (it == queue_family_properties.end()) {
    return bird::fail("Could not find a queue family supporting both graphics and presentation");
  }

  auto queue_index = static_cast<uint32_t>(std::distance(queue_family_properties.begin(), it));

  float queue_priority = 1.0f;
  vk::DeviceQueueCreateInfo device_queue_create_info{
      .flags = vk::DeviceQueueCreateFlags(),
      .queueFamilyIndex = queue_index,
      .queueCount = 1,
      .pQueuePriorities = &queue_priority
  };

  vk::StructureChain<
      vk::PhysicalDeviceFeatures2,
      vk::PhysicalDeviceVulkan11Features,
      vk::PhysicalDeviceVulkan13Features,
      vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
  > feature_chain(
      vk::PhysicalDeviceFeatures2{},
      vk::PhysicalDeviceVulkan11Features{ .shaderDrawParameters = true },
      vk::PhysicalDeviceVulkan13Features{ .dynamicRendering = true },
      vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT{ .extendedDynamicState = true }
  );

  std::vector<const char*> required_device_extensions = { vk::KHRSwapchainExtensionName };

  vk::DeviceCreateInfo device_create_info{
      .pNext = &feature_chain.get<vk::PhysicalDeviceFeatures2>(),
      .flags = vk::DeviceCreateFlags(),
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &device_queue_create_info,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = static_cast<uint32_t>(required_device_extensions.size()),
      .ppEnabledExtensionNames = required_device_extensions.data(),
      .pEnabledFeatures = nullptr
  };

  vk::PhysicalDevice raw_phys_dev = *physical_device;
  auto dev_res = raw_phys_dev.createDevice(device_create_info);
  if (dev_res.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create logical device");
  }
  logical_device = vk::raii::Device(physical_device, dev_res.value);

  // getQueue returns the vk::Queue directly, no ResultValue because it cannot fail
  vk::Queue raw_queue = (*logical_device).getQueue(queue_index, 0);
  graphics_queue = vk::raii::Queue(logical_device, raw_queue);

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
  auto r_load_vert_shader_module = load_shader_module("src/bird_core/rendering/shaders/triangle.vert.glsl");
  if (!r_load_vert_shader_module) return bird::fail(r_load_vert_shader_module.error());

  auto r_load_frag_shader_module = load_shader_module("src/bird_core/rendering/shaders/triangle.frag.glsl");
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
    .pName = "fragMain"
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
  vk::PipelineInputAssemblyStateCreateInfo inputAssembly{.topology = vk::PrimitiveTopology::eTriangleList};

  // viewport and scissor
  vk::Viewport viewport{0.0f, 0.0f, static_cast<float>(swap_chain_extent.width), static_cast<float>(swap_chain_extent.height), 0.0f, 1.0f};
  vk::Rect2D scissor{vk::Offset2D{ 0, 0 }, swap_chain_extent};

  // viewport create info
  vk::PipelineViewportStateCreateInfo viewportState{.viewportCount = 1, .pViewports = &viewport, .scissorCount = 1, .pScissors = &scissor};

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

  return bird::ok();
}

Result<std::unique_ptr<vk::raii::ShaderModule>> VulkanRenderer::load_shader_module(const std::string filepath) const {
  ShaderCompiler shader_compiler;

  std::vector<uint32_t> vert_spirv = shader_compiler.compile_glsl_file_to_spirv(filepath);

  if (vert_spirv.empty()) {
    return bird::fail("Shader compilation failed: " + shader_compiler.get_last_error());
  }

  vk::ShaderModuleCreateInfo create_info{
      .codeSize = vert_spirv.size() * sizeof(uint32_t), // Size in bytes
      .pCode    = vert_spirv.data()                    // Pointer to uint32_t data
  };

  VkShaderModule raw_module = VK_NULL_HANDLE;
  VkResult result = vkCreateShaderModule(
      *logical_device,
      reinterpret_cast<const VkShaderModuleCreateInfo*>(&create_info),
      nullptr,
      &raw_module
  );

  if (result != VK_SUCCESS) {
    return bird::fail("Vulkan failed to create shader module for: " + filepath);
  }

  return std::make_unique<vk::raii::ShaderModule>(logical_device, raw_module);
}

// TODO
void VulkanRenderer::render() {}

Result<void> VulkanRenderer::shutdown() {
  return bird::ok();
}

void VulkanRenderer::resize_frame_buffer(int width, int height) {}
void VulkanRenderer::submit_quad(const RenderCommand render_command) {}
void VulkanRenderer::handle_window_resize(int width, int height) {}

} // namespace bird