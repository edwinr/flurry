#ifndef Flurry_ps3_ps3texture_h
#define Flurry_ps3_ps3texture_h
#include <rsx/rsx.h>
#include <cstdint>

class PS3Texture {
    gcmTexture tex;
    uint8_t* buffer = nullptr;

   public:
    void init(uint32_t width, uint32_t height, const void* pixels);
    void destroy();
    void set(gcmContextData* context, uint8_t textureUnit);
    inline operator bool() const {
        return buffer != nullptr;
    }
    ~PS3Texture();
};

#endif
