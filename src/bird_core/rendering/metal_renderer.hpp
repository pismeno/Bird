#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "irenderer.hpp"
#include <QuartzCore/CAMetalLayer.hpp>

namespace MTL {
class RenderCommandEncoder;
}

namespace bird {

class MetalRenderer : public IRenderer {
public:
    void init() override;
    void run() override;
    void shutdown() override;

private:
    void init_device();
    void init_window();

    void create_triangle();

    void create_default_library();
    void create_command_queue();
    void create_render_pipeline();

    void encode_render_command(MTL::RenderCommandEncoder* render_command_encoder);
    void send_render_command();
    void draw();

    static void frame_buffer_size_callback(GLFWwindow *window, int width, int height);
    void resize_frame_buffer(int width, int height);

    struct Impl;
    Impl* metal_backend_ = nullptr;
};

}