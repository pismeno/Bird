#pragma once

#include <rendering/irenderer.hpp>

#ifdef BIRD_PLATFORM_WINDOWS
#include <windows.h>
#endif

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

  friend class IRenderer;

 public:
  static constexpr inline uint32_t MAX_FRAMES_IN_FLIGHT = 2;

  Result<void> attach_window(void* native_window_handle, uint32_t window_width, uint32_t window_height) override;
  Result<void> init(RenderingContext& context) override;

  void begin_frame() override;
  void begin_pass(const ViewData& view_data) override;
  void submit(const RenderCommand render_command) override;
  void end_pass() override;
  void end_frame() override;

  Result<void> resize_framebuffer(uint32_t width, uint32_t height) override;
  bool needs_framebuffer_resize() const noexcept override;
  bool is_context_lost() const noexcept override;

 private:
  explicit VulkanRenderer();
  ~VulkanRenderer();

  // initialization methods
  Result<void> create_texture_image();
  Result<void> create_texture_image_view();
  Result<void> create_texture_sampler();

  //Result<void> create_graphics_pipeline();

  Result<void> create_command_pool();
  Result<void> create_vertex_buffer();
  Result<void> create_index_buffer();
  Result<void> create_uniform_buffers();
  Result<void> create_descriptor_pool();
  Result<void> create_command_buffers();
  Result<void> create_sync_objects();

  // single-time command buffer helpers:
  Result<vk::raii::CommandBuffer> begin_single_time_commands();
  void end_single_time_commands(vk::raii::CommandBuffer& command_buffer);

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

  // rendering helper methods
  Result<void> record_command_buffer();
  void update_ubo(const ViewData& view_data);
  inline void move_to_next_frame();

  void transition_image_layout(
      vk::raii::CommandBuffer& cmd_buffer,
      vk::Image                image,
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

  vma::raii::Buffer vertex_buffer = nullptr;
  vma::raii::Buffer index_buffer = nullptr;

  std::vector<vma::raii::Buffer> uniform_buffers;
  std::vector<void*> uniform_buffers_mapped;

  vk::raii::DescriptorPool descriptor_pool = nullptr;
  std::vector<vk::raii::DescriptorSet> descriptor_sets;
  vk::raii::CommandPool command_pool = nullptr;
  std::vector<vk::raii::CommandBuffer> command_buffers;

  vma::raii::Image texture_image = nullptr;
  vk::raii::ImageView texture_image_view = nullptr;
  vk::raii::Sampler texture_sampler = nullptr;

  std::vector<vk::raii::Semaphore> present_complete_semaphores;
  std::vector<vk::raii::Semaphore> render_finished_semaphores;
  std::vector<vk::raii::Fence> draw_fences;

  // rendering states
  bool is_context_lost_ = false;
  bool needs_framebuffer_resize_ = false;
  uint32_t current_image_index_ = 0;
  uint32_t current_frame_ = 0;

  // bird systems
  AssetManager* asset_manager;

  const std::vector<Vertex> vertices = {
      {{-0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
      {{ 0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
      {{-0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
      {{ 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}};

  const std::vector<uint16_t> indices = {
      0, 3, 1,   // Top-Left -> Top-Right -> Bottom-Right (CW)
      0, 1, 2    // Top-Left -> Bottom-Right -> Bottom-Left (CW)
  };
};

} // namespace bird