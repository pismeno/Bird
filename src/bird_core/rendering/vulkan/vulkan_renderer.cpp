#include "rendering/vulkan/vulkan_renderer.hpp"

#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>

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

// TODO
void VulkanRenderer::render() {}

Result<void> VulkanRenderer::shutdown() {
  return bird::ok();
}

void VulkanRenderer::resize_frame_buffer(int width, int height) {}
void VulkanRenderer::submit_quad(const RenderCommand render_command) {}
void VulkanRenderer::handle_window_resize(int width, int height) {}

} // namespace bird