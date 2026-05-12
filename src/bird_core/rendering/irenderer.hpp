#pragma once

namespace bird {

class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual void init() = 0;
    virtual void run() = 0;
    virtual void shutdown() = 0;
};

} // namespace bird