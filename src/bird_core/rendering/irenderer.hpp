#pragma once

#include "../node_system/managers/transform/transforms.hpp"

namespace bird {

class Window;

class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual void init(Window& window) = 0;
    virtual void render() = 0;
    virtual void shutdown() = 0;
    virtual void submit_quad(const bird::node_system::managers::Transform2D& transform) = 0;
};

} // namespace bird