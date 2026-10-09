#include "vulkan_pipeline.hpp"

#include <memory>

#include "utils/result.hpp"
#include "rendering/vulkan/vulkan_context.hpp"
#include "rendering/vulkan/vulkan_swapchain.hpp"
#include "resources/asset_handle.hpp"
#include "resources/asset_types.hpp"
#include "resources/asset_manager.hpp"

namespace bird {

Result<std::unique_ptr<VulkanPipeline>> VulkanPipeline::create(VulkanSwapchain& swapchain, VulkanContext& vk_context, VulkanMaterialHandler& material_handler, AssetManager* asset_manager) {
  std::unique_ptr<VulkanPipeline> pipeline(new VulkanPipeline());

  auto r_create_descriptor_set_layout = pipeline->create_descriptor_set_layout(vk_context);
  if (!r_create_descriptor_set_layout) return bird::fail(r_create_descriptor_set_layout.error());

  auto r_create_graphics_pipeline = pipeline->create_graphics_pipeline(swapchain, vk_context, material_handler, asset_manager);
  if (!r_create_graphics_pipeline) return bird::fail(r_create_graphics_pipeline.error());

  return std::move(pipeline);
}

Result<void> VulkanPipeline::create_descriptor_set_layout(VulkanContext& vk_context) {
  vk::DescriptorSetLayoutBinding ubo_binding{
      .binding            = 0,
      .descriptorType     = vk::DescriptorType::eUniformBuffer,
      .descriptorCount    = 1,
      .stageFlags         = vk::ShaderStageFlagBits::eVertex,
      .pImmutableSamplers = nullptr
  };

  vk::DescriptorSetLayoutBinding ssbo_binding = {
      .binding = 1,
      .descriptorType = vk::DescriptorType::eStorageBuffer, // <--- Storage Buffer!
      .descriptorCount = 1,
      .stageFlags = vk::ShaderStageFlagBits::eVertex
  };

  std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {ubo_binding, ssbo_binding};

  vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_info = {
      .bindingCount = static_cast<uint32_t>(bindings.size()),
      .pBindings = bindings.data()
  };

  auto vkr_create_desc_set_layout = vk_context.get_logical_device().createDescriptorSetLayout(descriptor_set_layout_info);
  if (vkr_create_desc_set_layout.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create descriptor set layout");
  }

  descriptor_set_layout = std::move(vkr_create_desc_set_layout.value);
  return bird::ok();
}

template <std::derived_from<ShaderAsset> T>
Result<std::unique_ptr<vk::raii::ShaderModule>> load_shader_module(const std::string& filepath, VulkanContext& vk_context, AssetManager* asset_manager) {
  auto r_shader_asset = asset_manager->acquire<T>(filepath);
  if (!r_shader_asset) {
    return bird::fail(r_shader_asset.error());
  }

  AssetHandle<T> shader_handle = std::move(r_shader_asset).value();

  vk::ShaderModuleCreateInfo create_info{
      .codeSize = shader_handle->data.size(),      // Size in bytes
      .pCode    = shader_handle->as_32bit_words()  // Pointer to aligned uint32_t data
  };

  auto vkr_create_shader_module = vk_context.get_logical_device().createShaderModule(create_info);
  if (vkr_create_shader_module.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create shader module for: " + filepath);
  }

  return std::make_unique<vk::raii::ShaderModule>(std::move(vkr_create_shader_module.value));
}

Result<void> VulkanPipeline::create_graphics_pipeline(VulkanSwapchain& swapchain, VulkanContext& vk_context, VulkanMaterialHandler& material_handler, AssetManager* asset_manager) {
  // loading the shaders from files
  auto r_load_vert_shader_module = load_shader_module<VertexShaderAsset>("core://shaders/vertices.vert.spv", vk_context, asset_manager);
  if (!r_load_vert_shader_module) return bird::fail(r_load_vert_shader_module.error());

  auto r_load_frag_shader_module = load_shader_module<FragmentShaderAsset>("core://shaders/textures.frag.spv", vk_context, asset_manager);
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
  vk::PipelineVertexInputStateCreateInfo vertex_input_info{
      .vertexBindingDescriptionCount   = 0,
      .pVertexBindingDescriptions      = nullptr,
      .vertexAttributeDescriptionCount = 0,
      .pVertexAttributeDescriptions    = nullptr
  };

  // input assembly create info
  vk::PipelineInputAssemblyStateCreateInfo input_assembly{.topology = vk::PrimitiveTopology::eTriangleList};

  const vk::Extent2D& swap_chain_extent = swapchain.get_extent();
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
      .cullMode                = vk::CullModeFlagBits::eNone,
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
      .blendEnable    = vk::True,
      .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
      .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
      .colorBlendOp        = vk::BlendOp::eAdd,
      .srcAlphaBlendFactor = vk::BlendFactor::eOne,
      .dstAlphaBlendFactor = vk::BlendFactor::eZero,
      .alphaBlendOp        = vk::BlendOp::eAdd,
      .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
  };

  vk::PipelineColorBlendStateCreateInfo color_blending{
      .logicOpEnable = vk::False,
      .logicOp = vk::LogicOp::eCopy,
      .attachmentCount = 1,
      .pAttachments = &color_blend_attachment
  };

  std::array<vk::DescriptorSetLayout, 2> set_layouts = {
      descriptor_set_layout,                                        // Set 0: UBO
      material_handler.get_texture_descriptor_set_layout()          // Set 1: Bindless Textures
  };

  // pipeline layout create info
  vk::PipelineLayoutCreateInfo pipeline_layout_info{
      .setLayoutCount = static_cast<uint32_t>(set_layouts.size()),
      .pSetLayouts    = set_layouts.data(),
  };

  const vk::raii::Device& logical_device = vk_context.get_logical_device();

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
          .pColorAttachmentFormats = &swapchain.get_surface_format().format
      }
  };


  auto vkr_create_graphics_pipeline = logical_device.createGraphicsPipeline(nullptr, pipeline_create_info_chain.get<vk::GraphicsPipelineCreateInfo>());
  if (vkr_create_graphics_pipeline.result != vk::Result::eSuccess) {
    return bird::fail("Failed to create graphics pipeline");
  }
  graphics_pipeline = std::move(vkr_create_graphics_pipeline.value);

  return bird::ok();
}

void VulkanPipeline::bind(const vk::raii::CommandBuffer& command_buffer) const {
  command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphics_pipeline);
}

} // bird