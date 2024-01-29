#include "ps3texture.h"

void PS3Texture::init(uint32_t width, uint32_t height, const void* pixels) {
    buffer = (uint8_t*)rsxMemalign(128, width * height * 4);
    if (!buffer) {
        return;
    }

    auto src = (const uint8_t*)pixels;
    auto dst = buffer;
    for (uint32_t i = 0; i < width * height * 4; i += 4) {
        dst[i + 1] = *src++;
        dst[i + 2] = *src++;
        dst[i + 3] = *src++;
        dst[i + 0] = *src++;
    }

    uint32_t pitch = width * 4;
    uint32_t offset = 0;
    rsxAddressToOffset(buffer, &offset);

    tex.format = (GCM_TEXTURE_FORMAT_A8R8G8B8 | GCM_TEXTURE_FORMAT_LIN);
    tex.mipmap = 1;
    tex.dimension = GCM_TEXTURE_DIMS_2D;
    tex.cubemap = GCM_FALSE;
    tex.remap =
        ((GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_B_SHIFT) |
         (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_G_SHIFT) |
         (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_R_SHIFT) |
         (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_A_SHIFT) |
         (GCM_TEXTURE_REMAP_COLOR_B << GCM_TEXTURE_REMAP_COLOR_B_SHIFT) |
         (GCM_TEXTURE_REMAP_COLOR_G << GCM_TEXTURE_REMAP_COLOR_G_SHIFT) |
         (GCM_TEXTURE_REMAP_COLOR_R << GCM_TEXTURE_REMAP_COLOR_R_SHIFT) |
         (GCM_TEXTURE_REMAP_COLOR_A << GCM_TEXTURE_REMAP_COLOR_A_SHIFT));
    tex.width = width;
    tex.height = height;
    tex.depth = 1;
    tex.location = GCM_LOCATION_RSX;
    tex.pitch = pitch;
    tex.offset = offset;
}

void PS3Texture::destroy() {
    if (buffer) {
        rsxFree(buffer);
        buffer = nullptr;
    }
}

void PS3Texture::set(gcmContextData* context, uint8_t textureUnit) {
    rsxInvalidateTextureCache(context, GCM_INVALIDATE_TEXTURE);
    rsxLoadTexture(context, textureUnit, &tex);
}

PS3Texture::~PS3Texture() {
    destroy();
}
