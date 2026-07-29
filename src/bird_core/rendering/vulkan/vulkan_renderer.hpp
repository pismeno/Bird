#pragma once

#include "rendering/irenderer.hpp"

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

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

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
  Result<void> create_swap_chain();
  Result<void> create_image_views();
  Result<void> create_graphics_pipeline();

  vk::SurfaceFormatKHR choose_swap_surface_format(std::vector<vk::SurfaceFormatKHR> const &availableFormats);
  vk::PresentModeKHR choose_swap_present_mode(std::vector<vk::PresentModeKHR> const &availablePresentModes);
  vk::Extent2D choose_swap_extent(vk::SurfaceCapabilitiesKHR const &capabilities);
  uint32_t choose_swap_min_image_count(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);

  bool is_physical_device_suitable(const vk::PhysicalDevice& physical_device) const;

  Result<std::unique_ptr<vk::raii::ShaderModule>> load_shader_module(const std::string filepath) const;

  void* native_window_handle = nullptr;
  uint32_t window_width = 0;
  uint32_t window_height = 0;

  vk::raii::Context context;

  vk::raii::Instance instance = nullptr;
  vk::raii::PhysicalDevice physical_device = nullptr;
  vk::raii::Device logical_device = nullptr;
  vk::raii::Queue graphics_queue = nullptr;
  vk::raii::SurfaceKHR surface = nullptr;
  vk::raii::SwapchainKHR swap_chain = nullptr;

  std::vector<vk::Image> swap_chain_images;
  vk::SurfaceFormatKHR   swap_chain_surface_format;
  vk::Extent2D           swap_chain_extent;

  vk::raii::PipelineLayout pipeline_layout = nullptr;
  vk::raii::Pipeline graphics_pipeline = nullptr;

  std::vector<vk::raii::ImageView> swap_chain_image_views;
};

} // namespace bird