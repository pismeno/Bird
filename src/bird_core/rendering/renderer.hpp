#pragma once

#include <memory>

#include "irenderer.hpp"

namespace bird {

enum class RendererType {
    Metal,
    Vulkan,
    DirectX
};

class Renderer {
public:
    ~Renderer();

    void create_renderer(RendererType type);
    void init();

private:
    std::unique_ptr<IRenderer> renderer_;

};

}