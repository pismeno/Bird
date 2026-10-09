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

#include <rendering/vulkan/quad.hpp>
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
  static constexpr inline size_t MAX_QUADS = 100000;

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
    vk::raii::Fence in_flight_fence = nullptr;

    vk::raii::CommandBuffer command_buffer = nullptr;

    vma::raii::Buffer uniform_buffer = nullptr;
    void* uniform_buffer_mapped = nullptr;
    vma::raii::Buffer quad_buffer = nullptr;
    Quad* quad_buffer_mapped = nullptr;

    vk::raii::DescriptorSet descriptor_set = nullptr;
  };

  Result<void> create_command_pool();
  Result<void> create_frame_data();

  Result<void> record_command_buffer(FrameData& frame);
  void update_ubo(const ViewData& view_data, FrameData& frame);
  void update_quad_ssbo(FrameData& frame);
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

  vk::raii::CommandPool command_pool = nullptr;

  std::vector<FrameData> frames;
  std::vector<vk::raii::Semaphore> render_finished_semaphores;

  // rendering states
  bool is_context_lost_ = false;
  bool needs_framebuffer_resize_ = false;
  uint32_t current_image_index_ = 0;
  uint32_t current_frame_ = 0;

  // bird systems
  AssetManager* asset_manager;

  const std::vector<bird::Quad> quads = {
      // 1. Standard Square: 100x100 at Position (100, 100)
      {
          glm::mat3x2(
              glm::vec2(100.0f, 0.0f),  // Column 0: Scale X = 100, Shear Y = 0
              glm::vec2(0.0f, 100.0f),  // Column 1: Shear X = 0,   Scale Y = 100
              glm::vec2(100.0f, 100.0f) // Column 2: Position X = 100, Position Y = 100
          ),
          0, // material_index
      },

      // 2. Wide Rectangle with HORIZONTAL SHEAR (Skew X)
      // The top of the rectangle will be pushed 100 pixels to the right
      {
          glm::mat3x2(
              glm::vec2(250.0f, 0.0f),  // Column 0: Scale X = 250, Shear Y = 0
              glm::vec2(100.0f, 50.0f), // Column 1: SHEAR X = 100, Scale Y = 50
              glm::vec2(300.0f, 100.0f)
          ),
          0,
      },

      // 3. Tall Rectangle with VERTICAL SHEAR (Skew Y)
      // The right side of the rectangle will be pulled 100 pixels down
      {
          glm::mat3x2(
              glm::vec2(50.0f, 100.0f), // Column 0: Scale X = 50, SHEAR Y = 100
              glm::vec2(50.0f, 200.0f),  // Column 1: Shear X = 0,  Scale Y = 200
              glm::vec2(100.0f, 300.0f)
          ),
          0,
      },

      // 4. Rotated Square: 100x100 rotated 45 degrees at Position (300, 300)
      // Formula: [Scale * cos, Scale * sin], [Scale * -sin, Scale * cos]
      {
          glm::mat3x2(
              glm::vec2(100.0f * std::cos(0.785398f),  100.0f * std::sin(0.785398f)),
              glm::vec2(100.0f * -std::sin(0.785398f), 100.0f * std::cos(0.785398f)),
              glm::vec2(300.0f, 300.0f)
          ),
          0,
      }
  };
};

} // bird