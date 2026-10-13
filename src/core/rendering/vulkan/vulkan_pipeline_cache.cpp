#include "vulkan_pipeline_cache.hpp"

namespace bird {

Result<VulkanPipeline*> VulkanPipelineCache::get(const AssetHandle<VertexShaderAsset>& vert, const AssetHandle<FragmentShaderAsset>& frag) {
  CombinedShaderKey key{vert.get_packed_id(), frag.get_packed_id()};
  auto it = pipelines.find(key);
  if (it != pipelines.end()) {
    return it->second.get();
  }

  VulkanPipelineOptions create_options{
    .swapchain = swapchain,
    .vk_context = vk_context,
    .material_handler = material_handler,
    .descriptor_set_layout = descriptor_set_layout,
    .vert_shader = vert,
    .frag_shader = frag,
  };

  auto r_pipeline = VulkanPipeline::create(create_options);
  if (!r_pipeline) return bird::fail(r_pipeline.error());
  pipelines[key] = std::move(r_pipeline).value();

  return pipelines[key].get();
}

} // bird