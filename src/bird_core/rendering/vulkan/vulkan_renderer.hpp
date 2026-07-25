#pragma once

#include "rendering/irenderer.hpp"

#include <vulkan/vulkan_raii.hpp>

#include <cstdint>

#include "utils/result.hpp"
#include "rendering/render_command.hpp"

namespace bird {

class VulkanRenderer : public IRenderer {
 public:
  Result<void> attach_window(void* native_window_handle, uint32_t window_width, uint32_t window_height) override;
  Result<void> init() override;
  void render() override;
  Result<void> shutdown() override;
  void resize_frame_buffer(int width, int height);
  void submit_quad(const RenderCommand render_command) override;

  void handle_window_resize(int width, int height) override;
 private:
  Result<void> create_instance();
  Result<void> create_surface();
  Result<void> pick_physical_device();
  Result<void> create_logical_device();

  bool is_physical_device_suitable(const vk::PhysicalDevice& physical_device) const;

  void* native_window_handle = nullptr;

  vk::raii::Context vk_context;
  vk::raii::Instance vk_instance = nullptr;
  vk::raii::PhysicalDevice vk_physical_device = nullptr;
  vk::raii::Device vk_logical_device = nullptr;
  vk::raii::Queue graphics_queue = nullptr;
  vk::raii::SurfaceKHR surface = nullptr;
};

} // bird