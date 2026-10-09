#include "vulkan_context.hpp"

#include <memory>

#include <vulkan/vulkan_raii.hpp>
#ifdef BIRD_PLATFORM_WINDOWS
#include <windows.h>
#endif

#include "utils/result.hpp"

namespace bird {

Result<std::unique_ptr<VulkanContext>> VulkanContext::create(void* native_window_handle) {
  auto context = std::unique_ptr<VulkanContext>(new VulkanContext());

  auto r_create_instance = context->create_instance();
  if (!r_create_instance) return bird::fail(r_create_instance.error());

  auto r_create_surface = context->create_surface(native_window_handle);
  if (!r_create_surface) return bird::fail(r_create_surface.error());

  auto r_pick_physical_device = context->pick_physical_device();
  if (!r_pick_physical_device) return bird::fail(r_pick_physical_device.error());

  auto r_create_logical_device = context->create_logical_device();
  if (!r_create_logical_device) return bird::fail(r_create_logical_device.error());

  return std::move(context);
}

Result<void> VulkanContext::create_instance() {
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

  std::vector<const char*> layers;
#ifndef NDEBUG
  layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

  vk::InstanceCreateInfo create_info{
      .pApplicationInfo = &app_info,
      .enabledLayerCount = static_cast<uint32_t>(layers.size()),
      .ppEnabledLayerNames = layers.data(),
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data()
  };

  auto vkr_create = vk::createInstance(create_info);
  if (vkr_create.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create Vulkan Instance");
  }

  vk_instance = vk::raii::Instance(context, vkr_create.value);

  return bird::ok();
}

Result<void> VulkanContext::create_surface(void* native_window_handle) {
  vk::Win32SurfaceCreateInfoKHR createInfo{
      .hinstance = GetModuleHandle(nullptr),
      .hwnd      = static_cast<HWND>(native_window_handle)
  };

  // Dereference RAII object to get the raw vk::Instance wrapper to call creation
  vk::Instance raw_instance = *vk_instance;
  auto surf_res = raw_instance.createWin32SurfaceKHR(createInfo);
  if (surf_res.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create Win32 Surface");
  }

  surface = vk::raii::SurfaceKHR(vk_instance, surf_res.value);
  return bird::ok();
}

Result<void> VulkanContext::pick_physical_device() {
  auto phys_devs_res = vk_instance.enumeratePhysicalDevices();
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
bool VulkanContext::is_physical_device_suitable(const vk::PhysicalDevice &physical_device) const {
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

  auto features = physical_device.getFeatures2<
      vk::PhysicalDeviceFeatures2,
      vk::PhysicalDeviceVulkan11Features,
      vk::PhysicalDeviceVulkan13Features,
      vk::PhysicalDeviceDescriptorIndexingFeatures,
      vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

  const auto & vulkan11Features =
      features.template get<vk::PhysicalDeviceVulkan11Features>();

  const auto & vulkan13Features =
      features.template get<vk::PhysicalDeviceVulkan13Features>();

  const auto & descriptorIndexingFeatures =
      features.template get<vk::PhysicalDeviceDescriptorIndexingFeatures>();

  const auto & extendedDynamicStateFeatures =
      features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

  bool supportsRequiredFeatures =
      vulkan11Features.shaderDrawParameters &&
      vulkan13Features.synchronization2 &&
      vulkan13Features.dynamicRendering &&
      descriptorIndexingFeatures.descriptorBindingPartiallyBound &&
      descriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind &&
      extendedDynamicStateFeatures.extendedDynamicState;

  return supportsVulkan1_3 &&
         supportsGraphics &&
         supportsAllRequiredExtensions &&
         supportsRequiredFeatures;
}

Result<void> VulkanContext::create_logical_device() {
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
  auto supported_features = physical_device.getFeatures2<
      vk::PhysicalDeviceFeatures2,
      vk::PhysicalDeviceVulkan11Features,
      vk::PhysicalDeviceVulkan13Features,
      vk::PhysicalDeviceDescriptorIndexingFeatures,
      vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

  const auto & supported_vulkan11 =
      supported_features.get<vk::PhysicalDeviceVulkan11Features>();

  const auto & supported_vulkan13 =
      supported_features.get<vk::PhysicalDeviceVulkan13Features>();

  const auto & supported_descriptor_indexing =
      supported_features.get<vk::PhysicalDeviceDescriptorIndexingFeatures>();

  const auto & supported_extended_dynamic_state =
      supported_features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

  if (!supported_vulkan11.shaderDrawParameters) {
    return bird::fail("Required Vulkan 1.1 feature shaderDrawParameters is not supported");
  }

  if (!supported_vulkan13.synchronization2) {
    return bird::fail("Required Vulkan 1.3 feature synchronization2 is not supported");
  }

  if (!supported_vulkan13.dynamicRendering) {
    return bird::fail("Required Vulkan 1.3 feature dynamicRendering is not supported");
  }

  if (!supported_descriptor_indexing.descriptorBindingPartiallyBound) {
    return bird::fail("Required descriptor indexing feature descriptorBindingPartiallyBound is not supported");
  }

  if (!supported_descriptor_indexing.descriptorBindingSampledImageUpdateAfterBind) {
    return bird::fail("Required descriptor indexing feature descriptorBindingSampledImageUpdateAfterBind is not supported");
  }

  if (!supported_extended_dynamic_state.extendedDynamicState) {
    return bird::fail("Required extended dynamic state feature is not supported");
  }

  vk::PhysicalDeviceVulkan11Features vulkan11_features{
      .shaderDrawParameters = true
  };

  vk::PhysicalDeviceVulkan12Features vulkan12_features{
    .shaderSampledImageArrayNonUniformIndexing = true,
    .descriptorBindingSampledImageUpdateAfterBind = true,
    .descriptorBindingPartiallyBound = true,
    .runtimeDescriptorArray = true,
  };

  vk::PhysicalDeviceVulkan13Features vulkan13_features{
      .synchronization2 = true,
      .dynamicRendering = true
  };

  vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features{
      .extendedDynamicState = true
  };

  vk::StructureChain<
      vk::PhysicalDeviceFeatures2,
      vk::PhysicalDeviceVulkan11Features,
      vk::PhysicalDeviceVulkan12Features,
      vk::PhysicalDeviceVulkan13Features,
      vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
  > feature_chain = {
      {},
      vulkan11_features,
      vulkan12_features,
      vulkan13_features,
      extended_dynamic_state_features
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

} // bird