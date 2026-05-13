#include "texture.hpp"

Texture2D::Texture2D(const char* filepath, MTL::Device* metal_device) {
    device = metal_device;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* image = stbi_load(filepath, &width, &height, &channels, STBI_rgb_alpha);
    assert(image != NULL);

    MTL::TextureDescriptor* texture_descriptor = MTL::TextureDescriptor::alloc()->init();
    texture_descriptor->setPixelFormat(MTL::PixelFormatRGBA8Unorm);
    texture_descriptor->setWidth(width);
    texture_descriptor->setHeight(height);

    texture = device->newTexture(texture_descriptor);

    MTL::Region region = MTL::Region(0, 0, 0, width, height, 1);
    NS::UInteger bytes_per_row = 4 * width;

    texture->replaceRegion(region, 0, image, bytes_per_row);

    texture_descriptor->release();
    stbi_image_free(image);
}

Texture2D::~Texture2D() {
    texture->release();
}
