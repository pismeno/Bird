#include "rendering/vulkan/vulkan_material_handler.hpp"
#include <vulkan/vulkan_raii.hpp>

namespace bird {

Result<std::unique_ptr<VulkanMaterialHandler>> VulkanMaterialHandler::create(const VulkanContext& vk_context, const RenderingContext& context) {
  std::unique_ptr<VulkanMaterialHandler> handler(new VulkanMaterialHandler(context));

  auto r_create_layout = handler->create_descriptor_set_layout(vk_context);
  if (!r_create_layout) {
    return bird::fail("Failed to create Descriptor Set Layout");
  }

  auto r_create_pool = handler->create_descriptor_pool(vk_context);
  if (!r_create_pool) {
    return bird::fail("Failed to create Descriptor Pool");
  }

  auto r_create_tex_set = handler->create_texture_descriptor_set(vk_context);
  if (!r_create_tex_set) {
    return bird::fail("Failed to create Bindless Texture Set");
  }

  return std::move(handler);
}

Result<void> VulkanMaterialHandler::create_descriptor_set_layout(const VulkanContext& vk_context) {
  vk::DescriptorBindingFlags binding_flags =
      vk::DescriptorBindingFlagBits::eUpdateAfterBind |
      vk::DescriptorBindingFlagBits::ePartiallyBound;

  vk::DescriptorSetLayoutBindingFlagsCreateInfo binding_flags_info{
      .bindingCount = 1,
      .pBindingFlags = &binding_flags
  };

  vk::DescriptorSetLayoutBinding binding{
      .binding = 0,
      .descriptorType = vk::DescriptorType::eCombinedImageSampler,
      .descriptorCount = 65536, // TODO: query physicalDeviceProperties limits
      .stageFlags = vk::ShaderStageFlagBits::eFragment,
  };

  vk::DescriptorSetLayoutCreateInfo create_info{
      .pNext = &binding_flags_info,
      .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
      .bindingCount = 1,
      .pBindings = &binding
  };

  auto vkr_create = vk_context.get_logical_device().createDescriptorSetLayout(create_info);
  if (vkr_create.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create descriptor set layout");
  }
  texture_descriptor_set_layout = std::move(vkr_create.value);

  return bird::ok();
}

Result<void> VulkanMaterialHandler::create_descriptor_pool(const VulkanContext& vk_context) {
  vk::DescriptorPoolSize pool_size{vk::DescriptorType::eCombinedImageSampler, 65536};

  vk::DescriptorPoolCreateInfo create_info{
      .flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind | vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = 1,
      .poolSizeCount = 1,
      .pPoolSizes = &pool_size
  };

  auto vkr_create = vk_context.get_logical_device().createDescriptorPool(create_info);
  if (vkr_create.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create descriptor pool");
  }
  descriptor_pool = std::move(vkr_create.value);

  return bird::ok();
}

Result<void> VulkanMaterialHandler::create_texture_descriptor_set(const VulkanContext& vk_context) {
  vk::DescriptorSetLayout raw_layout = *texture_descriptor_set_layout;

  vk::DescriptorSetAllocateInfo alloc_info{
      .descriptorPool = *descriptor_pool,
      .descriptorSetCount = 1,
      .pSetLayouts = &raw_layout
  };

  auto vkr_create = vk_context.get_logical_device().allocateDescriptorSets(alloc_info);
  if (vkr_create.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create texture descriptor set");
  }
  bindless_texture_set = std::move(vkr_create.value[0]);

  return bird::ok();
}

} // bird