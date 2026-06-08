#pragma once

#include "rendering/render_command.hpp"

namespace bird {

class Window;

class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual void init(Window& window) = 0;
    virtual void render() = 0;
    virtual void shutdown() = 0;
    virtual void submit_quad(const RenderCommand render_command) = 0;
};

} // namespace bird