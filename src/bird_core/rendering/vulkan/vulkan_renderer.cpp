#include "rendering/vulkan/vulkan_renderer.hpp"

#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>

#include <vulkan/vulkan_raii.hpp>

#include "utils/result.hpp"

namespace bird {

Result<void> VulkanRenderer::attach_window(void* native_window_handle, uint32_t window_width, uint32_t window_height) {
  this->native_window_handle = native_window_handle;
}

Result<void> VulkanRenderer::init() {
  auto r_create_instance = create_instance();
  if (!r_create_instance) return r_create_instance;

  auto r_pick_physical_device = pick_physical_device();
  if (!r_pick_physical_device) return r_pick_physical_device;

  auto r_create_logical_device = create_logical_device();
  if (!r_create_logical_device) return r_create_logical_device;

  //vkDestroyInstance(instance, nullptr);

  return bird::ok();
}

Result<void> VulkanRenderer::create_instance() {
  vk::ApplicationInfo app_info{};
  app_info.pApplicationName = "Hello Triangle";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "BirdEngine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  vk::InstanceCreateInfo create_info{};
  create_info.pApplicationInfo = &app_info;

  // TODO: Implement validation layers
  create_info.enabledLayerCount = 0;
  create_info.ppEnabledLayerNames = nullptr;

  std::vector<const char*> extensions = { "VK_KHR_surface" };
#ifdef BIRD_PLATFORM_WINDOWS
  extensions.push_back("VK_KHR_win32_surface");
#endif
#ifdef BIRD_PLATFORM_LINUX
  extensions.push_back("VK_KHR_xcb_surface");
#endif

  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();

  vk_instance = vk::raii::Instance(vk_context, create_info);

  return bird::ok();
}

Result<void> VulkanRenderer::create_surface() {
  vk::Win32SurfaceCreateInfoKHR createInfo{.hinstance = GetModuleHandle(nullptr),
                                           .hwnd      = native_window_handle};

  surface = vk_instance.createWin32SurfaceKHR(createInfo);
}

Result<void> VulkanRenderer::pick_physical_device() {
  std::vector<vk::raii::PhysicalDevice> physicalDevices = vk_instance.enumeratePhysicalDevices();
  auto const devIter = std::ranges::find_if(physicalDevices, [&]( auto const & physicalDevice) {
    return is_physical_device_suitable(physicalDevice);
  });

  if (devIter == physicalDevices.end()) {
    return bird::fail("No suitable physical device found");
  }

  vk_physical_device = *devIter;

  return bird::ok();
}

bool VulkanRenderer::is_physical_device_suitable(const vk::PhysicalDevice &physical_device) const {
  std::vector<const char*> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};

  bool supportsVulkan1_3 = physical_device.getProperties().apiVersion >= vk::ApiVersion13;

  // Check if any of the queue families support graphics operations
  auto queueFamilies    = physical_device.getQueueFamilyProperties();
  bool supportsGraphics = std::ranges::any_of( queueFamilies, []( auto const & qfp ) { return !!( qfp.queueFlags & vk::QueueFlagBits::eGraphics ); } );

  // Check if all required physical_device extensions are available
  auto availableDeviceExtensions = physical_device.enumerateDeviceExtensionProperties();
  bool supportsAllRequiredExtensions =
      std::ranges::all_of( requiredDeviceExtension,
                           [&availableDeviceExtensions]( auto const & requiredDeviceExtension )
                           {
                             return std::ranges::any_of( availableDeviceExtensions,
                                                         [requiredDeviceExtension]( auto const & availableDeviceExtension )
                                                         { return strcmp( availableDeviceExtension.extensionName, requiredDeviceExtension ) == 0; } );
                           } );

  // Check if the physical_device supports the required features (shader draw parameters, dynamic rendering and extended dynamic state)
  auto features                 = physical_device.template getFeatures2<vk::PhysicalDeviceFeatures2,
      vk::PhysicalDeviceVulkan11Features,
      vk::PhysicalDeviceVulkan13Features,
      vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
  bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                                  features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                                  features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

  // Return true if the physicalDevice meets all the criteria
  return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
}

Result<void> VulkanRenderer::create_logical_device() {
  auto queue_family_properties = vk_physical_device.getQueueFamilyProperties();

  auto it = std::ranges::find_if(queue_family_properties, [this, index = 0u](const auto& qfp) mutable {
    uint32_t current_index = index++;
    bool supports_graphics = static_cast<bool>(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
    bool supports_present  = vk_physical_device.getSurfaceSupportKHR(current_index, *surface);

    return supports_graphics && supports_present;
  });

  if (it == queue_family_properties.end()) {
    return bird::fail("Could not find a queue family supporting both graphics and presentation");
  }

  auto queue_index = static_cast<uint32_t>(std::distance(queue_family_properties.begin(), it));

  // Queue Creation Info
  float queue_priority = 1.0f;
  vk::DeviceQueueCreateInfo device_queue_create_info(
      vk::DeviceQueueCreateFlags(),
      queue_index,
      1,               // queueCount
      &queue_priority  // pQueuePriorities
  );

  // Feature chain configuration
  vk::StructureChain<
      vk::PhysicalDeviceFeatures2,
      vk::PhysicalDeviceVulkan11Features,
      vk::PhysicalDeviceVulkan13Features,
      vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
  > feature_chain = {
      {},
      {.shaderDrawParameters = true},
      {.dynamicRendering = true},
      {.extendedDynamicState = true}
  };

  std::vector<const char*> required_device_extensions = { vk::KHRSwapchainExtensionName };

  // Device Creation Info
  vk::DeviceCreateInfo device_create_info(
      vk::DeviceCreateFlags(),
      1,
      &device_queue_create_info,
      0, nullptr,
      static_cast<uint32_t>(required_device_extensions.size()),
      required_device_extensions.data(),
      nullptr,
      &feature_chain.get<vk::PhysicalDeviceFeatures2>()
  );

  // Instantiation & Queue retrieval
  vk_logical_device = vk::raii::Device(vk_physical_device, device_create_info);
  graphics_queue    = vk::raii::Queue(vk_logical_device, queue_index, 0);

  return bird::ok();
}

}