#pragma once

#include "irenderer.hpp"
#include "windowing/iwindow.hpp"
#include <QuartzCore/CAMetalLayer.hpp>
#include "vertex.hpp"
#include "texture.hpp"
#include <filesystem>
#include <stb/stb_image.h>
#include "node_system/render_gatherer.hpp"


namespace MTL {
class RenderCommandEncoder;
}

struct GLFWwindow;

namespace bird {


class MetalRenderer : public IRenderer {
public:
  void init(IWindow& window) override;
  void render() override;
  void shutdown() override;
  void resizeFrameBuffer(int width, int height);
  void submit_quad(const RenderCommand render_command) override;

private:
  void init_device();
  void init_window(IWindow& window);


  void create_default_library();
  void create_command_queue();
  void create_render_pipeline();
  void create_texture();

  void encode_render_command(MTL::RenderCommandEncoder* render_command_encoder);
  void send_render_command();
  void draw();

  struct Impl;
  Impl* metal_backend_ = nullptr;

  Texture2D* texture = nullptr;
};

}