#pragma once

#include "rendering/irenderer.hpp"

#include <vulkan/vulkan.hpp>

#include "utils/result.hpp"
#include "windowing/iwindow.hpp"
#include "rendering/render_command.hpp"

namespace bird {

class VulkanRenderer : public IRenderer {
 public:
  Result<void> init(IWindow& window) override;
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