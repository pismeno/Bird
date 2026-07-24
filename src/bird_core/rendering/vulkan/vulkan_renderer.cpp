#include "rendering/vulkan/vulkan_renderer.hpp"

#include <vulkan/vulkan.hpp>

#include "utils/result.hpp"

namespace bird {

Result<void> VulkanRenderer::init() {
  auto result = create_instance();
  if (!result) return result;

  vkDestroyInstance(vk_instance, nullptr);

  return bird::ok();
}

Result<void> VulkanRenderer::create_instance() {
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Hello Triangle";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "BirdEngine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = 0;
  createInfo.ppEnabledExtensionNames = nullptr;
  createInfo.enabledLayerCount = 0;
  createInfo.ppEnabledLayerNames = nullptr;

  if (vkCreateInstance(&createInfo, nullptr, &vk_instance) != VK_SUCCESS) {
    return bird::fail("failed to create Vulkan instance");
  }

  return bird::ok();
}

}