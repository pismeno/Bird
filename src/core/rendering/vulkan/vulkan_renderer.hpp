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

#include <rendering/vulkan/vertex.hpp>
#include <rendering/vulkan/vulkan_context.hpp>
#include <rendering/vulkan/vulkan_memory_allocator.hpp>
#include <rendering/vulkan/vulkan_swapchain.hpp>
#include <rendering/vulkan/vulkan_pipeline.hpp>
#include <rendering/vulkan/vulkan_material_handler.hpp>
#include <rendering/rendering_context.hpp>
#include <resources/render_command.hpp>
#include <resources/asset_handle.hpp>
#include <resources/asset_manager.hpp>
#include <resources/asset_types.hpp>
#include <utils/result.hpp>

namespace bird {

class VulkanRenderer : public IRenderer {
 public:
  ~VulkanRenderer();

  static constexpr inline uint32_t MAX_FRAMES_IN_FLIGHT = 2;

  void begin_frame() override;
  void begin_pass(const ViewData& view_data) override;
  void submit(const RenderCommand render_command) override;
  void end_pass() override;
  void end_frame() override;

  Result<void> resize_framebuffer(uint32_t width, uint32_t height) override;
  bool needs_framebuffer_resize() const noexcept override;
  bool is_context_lost() const noexcept override;

  static Result<std::unique_ptr<VulkanRenderer>> create(RenderingContext& context, void* native_window_handle, uint32_t window_width, uint32_t window_height);
 private:
  explicit VulkanRenderer() = default;

  struct FrameData {
    vk::raii::Semaphore image_available_semaphore = nullptr;
    vk::raii::Fence     in_flight_fence           = nullptr;
    vk::raii::CommandBuffer command_buffer        = nullptr;

    vma::raii::Buffer   uniform_buffer            = nullptr;
    void*               uniform_buffer_mapped     = nullptr;

    vk::raii::DescriptorSet descriptor_set        = nullptr;
  };

  Result<void> create_texture_image();
  Result<void> create_texture_image_view();
  Result<void> create_texture_sampler();

  Result<void> create_command_pool();
  Result<void> create_vertex_buffer();
  Result<void> create_index_buffer();
  Result<void> create_frame_data();

  Result<vk::raii::CommandBuffer> begin_single_time_commands();
  void end_single_time_commands(vk::raii::CommandBuffer& command_buffer);

  Result<void> record_command_buffer(FrameData& frame);
  void update_ubo(const ViewData& view_data, FrameData& frame);
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

  std::unique_ptr<VulkanContext> vk_context;
  std::unique_ptr<VulkanMemoryAllocator> memory_allocator;
  std::unique_ptr<VulkanSwapchain> swapchain;
  std::unique_ptr<VulkanMaterialHandler> material_handler;
  std::unique_ptr<VulkanPipeline> pipeline;

  vk::raii::DescriptorPool ubo_descriptor_pool = nullptr;

  vma::raii::Buffer vertex_buffer = nullptr;
  vma::raii::Buffer index_buffer = nullptr;

  vk::raii::CommandPool command_pool = nullptr;

  vma::raii::Image texture_image = nullptr;
  vk::raii::ImageView texture_image_view = nullptr;
  vk::raii::Sampler texture_sampler = nullptr;

  std::vector<FrameData> frames;
  std::vector<vk::raii::Semaphore> render_finished_semaphores;

  // rendering states
  bool is_context_lost_ = false;
  bool needs_framebuffer_resize_ = false;
  uint32_t current_image_index_ = 0;
  uint32_t current_frame_ = 0;

  // bird systems
  AssetManager* asset_manager;

  const std::vector<Vertex> vertices = {
      {{-0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, 0},
      {{ 0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, 0},
      {{-0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, 0},
      {{ 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, 0}};

  const std::vector<uint16_t> indices = {
      0, 3, 1,   // Top-Left -> Top-Right -> Bottom-Right (CW)
      0, 1, 2    // Top-Left -> Bottom-Right -> Bottom-Left (CW)
  };
};

} // namespace bird