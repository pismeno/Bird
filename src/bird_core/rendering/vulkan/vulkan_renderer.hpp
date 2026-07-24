#pragma once

#include "rendering/irenderer.hpp"

#include <vulkan/vulkan.hpp>

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

  VkInstance vk_instance;
};

} // bird