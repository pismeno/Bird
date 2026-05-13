#pragma once
#include <Metal/Metal.hpp>
#include <stb/stb_image.h>

class Texture2D {
public:
    Texture2D(const char* filepath, MTL::Device* metalDevice);
    ~Texture2D();
    MTL::Texture* texture;
    int width, height, channels;

private:
    MTL::Device* device;
};
