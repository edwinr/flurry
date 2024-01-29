#ifndef Flurry_ps3_graphics_h
#define Flurry_ps3_graphics_h

#include <rsx/rsx.h>

class Graphics {
   public:
    gcmContextData* context = nullptr;
    uint32_t displayWidth;
    uint32_t displayHeight;
    float displayAspectRatio;

    int init();
    void beginFrame();
    void endFrame();
    void finish();

   private:
    static const auto commandBufferSize = 512 * 1024;
    static const auto ioBufferAlignment = 1024 * 1024;
    static const auto ioBufferSize = 128 * 1024 * 1024;
    static const auto framebufferCount = 2;

    uint32_t currentFrameBuffer = 0;
    uint32_t nextLabelValue = 1;
    bool firstFrame = true;

    uint32_t colorBufferPitch;
    uint32_t colorBufferOffsets[framebufferCount];
    void* colorBuffers[framebufferCount];

    void* ioBuffer;

    int initVideo();
    void waitIdle();
    void setRenderTarget(uint32_t framebufferIndex);
};

#endif
