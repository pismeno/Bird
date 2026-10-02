#pragma once

#include <rendering/irenderer.hpp>

#ifdef BIRD_PLATFORM_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#endif

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_NO_EXCEPTIONS

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

#include <rendering/rendering_context.hpp>
#include <resources/render_command.hpp>
#include <resources/asset_handle.hpp>
#include <resources/asset_manager.hpp>
#include <resources/asset_types.hpp>
#include <utils/result.hpp>

namespace bird {

class VulkanRenderer : public IRenderer {
 public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  Result<void> attach_window(void* native_window_handle, uint32_t window_width, uint32_t window_height) override;
  Result<void> init(RenderingContext& context) override;
  Result<void> shutdown() override;

  void begin_frame() override;
  void begin_pass(const ViewData& view_data) override;
  void submit(const RenderCommand render_command) override;
  void end_pass() override;
  void end_frame() override;

  Result<void> resize_framebuffer(uint32_t width, uint32_t height) override;
  bool needs_framebuffer_resize() const noexcept override;
  bool is_context_lost() const noexcept override;
 private:
  // initialization methods
  Result<void> create_instance();
  Result<void> create_surface();
  Result<void> pick_physical_device();
  Result<void> create_logical_device();
  Result<void> create_vma();
  Result<void> create_swap_chain();
  Result<void> create_image_views();
  Result<void> create_descriptor_set_layout();
  Result<void> create_graphics_pipeline();
  Result<void> create_command_pool();
  Result<void> create_vertex_buffer();
  Result<void> create_index_buffer();
  Result<void> create_uniform_buffers();
  Result<void> create_command_buffer();
  Result<void> create_sync_objects();

  //initialization helper methods
  vk::SurfaceFormatKHR choose_swap_surface_format(std::vector<vk::SurfaceFormatKHR> const &availableFormats);
  vk::PresentModeKHR choose_swap_present_mode(std::vector<vk::PresentModeKHR> const &availablePresentModes);
  vk::Extent2D choose_swap_extent(vk::SurfaceCapabilitiesKHR const &capabilities);
  uint32_t choose_swap_min_image_count(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);

  [[nodiscard]] bool is_physical_device_suitable(const vk::PhysicalDevice& physical_device) const;

  template <std::derived_from<ShaderAsset> T>
  Result<std::unique_ptr<vk::raii::ShaderModule>> load_shader_module(const std::string& filepath) const {
    auto r_shader_asset = asset_manager->acquire<T>(filepath);
    if (!r_shader_asset) {
      return bird::fail(r_shader_asset.error());
    }

    AssetHandle<T> shader_handle = std::move(r_shader_asset).value();

    vk::ShaderModuleCreateInfo create_info{
        .codeSize = shader_handle->data.size(),      // Size in bytes
        .pCode    = shader_handle->as_32bit_words()  // Pointer to aligned uint32_t data
    };

    auto vkr_create_shader_module = logical_device.createShaderModule(create_info);
    if (vkr_create_shader_module.result != vk::Result::eSuccess) {
      return bird::fail("Failed to create shader module for: " + filepath);
    }

    return std::make_unique<vk::raii::ShaderModule>(std::move(vkr_create_shader_module.value));
  }

  template <typename T>
  Result<void> create_buffer(vk::BufferUsageFlagBits usage, const std::vector<T>& src_data, vma::raii::Buffer& dest_buffer) {
    return create_buffer(usage, sizeof(src_data[0]) * src_data.size(), src_data, dest_buffer);
  }

  template <typename T>
  Result<void> create_buffer(vk::BufferUsageFlagBits usage, vk::DeviceSize size, const std::vector<T>& src_data, vma::raii::Buffer& dest_buffer) {
    vk::BufferCreateInfo buffer_info{
        .size        = size,
        .usage       = usage,
        .sharingMode = vk::SharingMode::eExclusive
    };

    vma::AllocationCreateInfo alloc_info{
        .flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite |
                 vma::AllocationCreateFlagBits::eMapped,
        .usage = vma::MemoryUsage::eAuto
    };

    auto vkr_create_buffer = vma_allocator.createBuffer(buffer_info, alloc_info);
    if (vkr_create_buffer.result != vk::Result::eSuccess) {
      return bird::fail("Failed to create and allocate Buffer via VMA-Hpp");
    }

    dest_buffer = std::move(vkr_create_buffer.value);

    const vma::raii::Allocation& allocation = dest_buffer.getAllocation();
    void* data = allocation.getInfo().pMappedData;

    memcpy(data, src_data.data(), buffer_info.size);

    return bird::ok();
  }

  // other vulkan methods
  Result<void> recreate_swap_chain();

  // rendering helper methods
  Result<void> record_command_buffer(const uint32_t image_index);

  // other helper methods
  Result<std::unique_ptr<vk::raii::ShaderModule>> load_shader_module(const std::string& filepath) const;

  void transition_image_layout(
      uint32_t                imageIndex,
      vk::ImageLayout         old_layout,
      vk::ImageLayout         new_layout,
      vk::AccessFlags2        src_access_mask,
      vk::AccessFlags2        dst_access_mask,
      vk::PipelineStageFlags2 src_stage_mask,
      vk::PipelineStageFlags2 dst_stage_mask);

  // window members that hold values for surface initialization
  void* native_window_handle = nullptr;
  uint32_t window_width = 0;
  uint32_t window_height = 0;

  // vulkan members
  vk::raii::Context context;

  vk::raii::Instance vk_instance = nullptr;
  vk::raii::PhysicalDevice physical_device = nullptr;
  vk::raii::Device logical_device = nullptr;
  uint32_t queue_index = ~0;
  vk::raii::Queue graphics_queue = nullptr;
  vk::raii::SurfaceKHR surface = nullptr;
  vk::raii::SwapchainKHR swap_chain = nullptr;

  vma::raii::Allocator vma_allocator = nullptr;

  std::vector<vk::Image> swap_chain_images;
  vk::SurfaceFormatKHR   swap_chain_surface_format;
  vk::Extent2D           swap_chain_extent;

  vk::raii::DescriptorSetLayout descriptor_set_layout = nullptr;
  vk::raii::PipelineLayout pipeline_layout = nullptr;
  vk::raii::Pipeline graphics_pipeline = nullptr;

  vma::raii::Buffer vertex_buffer = nullptr;
  vma::raii::Buffer index_buffer = nullptr;

  vk::raii::CommandPool command_pool = nullptr;
  vk::raii::CommandBuffer command_buffer = nullptr;

  std::vector<vk::raii::ImageView> swap_chain_image_views;

  vk::raii::Semaphore present_complete_semaphore = nullptr;
  vk::raii::Semaphore render_finished_semaphore  = nullptr;
  vk::raii::Fence     draw_fence                 = nullptr;

  // rendering states
  bool is_context_lost_ = false;
  bool needs_framebuffer_resize_ = false;
  uint32_t current_image_index_ = 0;

  // bird systems
  AssetManager* asset_manager;

  struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static vk::VertexInputBindingDescription getBindingDescription() {
      return {.binding = 0, .stride = sizeof(Vertex), .inputRate = vk::VertexInputRate::eVertex};
    }

    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
      return {{{.location = 0, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, pos)},
               {.location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, color)}}};
    }
  };

  const std::vector<Vertex> vertices = {
      {{-0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},
      {{0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
      {{-0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}},
      {{0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}};

  const std::vector<uint16_t> indices = {
      0, 3, 1,   // Top-Left -> Top-Right -> Bottom-Right (CW)
      0, 1, 2    // Top-Left -> Bottom-Right -> Bottom-Left (CW)
  };
};

} // namespace bird