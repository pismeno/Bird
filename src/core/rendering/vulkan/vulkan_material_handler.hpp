#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#include <vector>
#include <memory>

#include "rendering/vulkan/vulkan_context.hpp"
#include "rendering/vulkan/vulkan_memory_allocator.hpp"
#include "rendering/rendering_context.hpp"
#include "rendering/bindless_texture_registry.hpp"
#include "rendering/packed_material_cache.hpp"
#include "resources/material_types.hpp"
#include "resources/asset_handle.hpp"
#include "resources/asset_types.hpp"
#include "utils/result.hpp"

namespace bird {

class VulkanMaterialHandler {
 public:
  static inline constexpr size_t MATERIAL_STRIDE = 128;
  static inline constexpr size_t MAX_MATERIALS = 1000;

  ~VulkanMaterialHandler() = default;

  Result<void> update_texture_buffer();
  Result<void> prepare_materials(std::vector<MaterialID> material_ids);

  inline const vk::raii::DescriptorSetLayout& get_descriptor_set_layout() const { return descriptor_set_layout; }
  inline const vk::raii::DescriptorSet& get_descriptor_set() const { return descriptor_set; }
  Result<uint32_t> get_material_index(MaterialID material_id) const;

  static Result<std::unique_ptr<VulkanMaterialHandler>> create(const VulkanContext& vk_context, VulkanMemoryAllocator& memory_allocator, const RenderingContext& context);
 private:
  struct GpuTexture {
    vma::raii::Image image = nullptr;
    vk::raii::ImageView view = nullptr;
  };

  explicit VulkanMaterialHandler(
      const VulkanContext& vk_context,
      VulkanMemoryAllocator& memory_allocator,
      const RenderingContext& context)
      : vk_context(vk_context),
        memory_allocator(memory_allocator),
        bindless_texture_registry(context.bindless_texture_registry),
        packed_material_cache(context.packed_material_cache) {}

  Result<void> create_descriptor_set_layouts();
  Result<void> create_descriptor_pool();
  Result<void> create_descriptor_set();
  Result<void> create_default_sampler();
  Result<void> create_command_pool();
  Result<void> create_material_buffer();

  /// Allocates a primary command buffer from command_pool and begins it with eOneTimeSubmit.
  [[nodiscard]] Result<vk::raii::CommandBuffer> allocate_upload_command_buffer();
  /// Creates a 2D optimal tiled image via VMA with dedicated allocation.
  [[nodiscard]] Result<vma::raii::Image> create_texture_image(const AssetHandle<ImageAsset>& asset);
  /// Creates a standard 2D color aspect image view for the uploaded texture.
  [[nodiscard]] Result<vk::raii::ImageView> create_texture_image_view(vk::Image image);

  vk::raii::DescriptorSetLayout descriptor_set_layout = nullptr;
  vk::raii::DescriptorPool descriptor_pool = nullptr;
  vk::raii::DescriptorSet descriptor_set = nullptr;

  vma::raii::Buffer material_buffer = nullptr;
  void* material_buffer_mapped = nullptr;

  vk::raii::Sampler default_sampler = nullptr;
  vk::raii::CommandPool command_pool = nullptr;

  // Keeps textures alive; indexed by their bindless slot index
  std::vector<GpuTexture> allocated_textures;

  std::vector<MaterialID> allocated_materials;
  std::unordered_map<MaterialID, uint32_t> material_indices;
  uint32_t next_unused_material_index{0};

  const VulkanContext& vk_context;
  VulkanMemoryAllocator& memory_allocator;
  BindlessTextureRegistry* bindless_texture_registry;
  PackedMaterialCache* packed_material_cache;
};

} // bird