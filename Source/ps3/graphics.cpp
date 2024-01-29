#include "Graphics.h"
#include <malloc.h>
#include <ppu-types.h>
#include <sysutil/video.h>
#include <unistd.h>
#include <cstdio>
#include "graphics.h"

static uint32_t findResolutionId() {
    const uint32_t resolutionIds[] = {
        VIDEO_RESOLUTION_1080, VIDEO_RESOLUTION_720, VIDEO_RESOLUTION_480,
        VIDEO_RESOLUTION_576};
    const size_t resolutionIdCount =
        sizeof(resolutionIds) / sizeof(resolutionIds[0]);

    int resolutionId = VIDEO_RESOLUTION_UNDEFINED;
    for (size_t i = 0; i < resolutionIdCount; i++) {
        if (videoGetResolutionAvailability(VIDEO_PRIMARY, resolutionIds[i],
                                           VIDEO_ASPECT_AUTO, 0) == 1) {
            resolutionId = resolutionIds[i];
            break;
        }
    }

    return resolutionId;
}

int Graphics::initVideo() {
    auto resolutionId = findResolutionId();

    videoResolution resolution;
    if (videoGetResolution(resolutionId, &resolution)) {
        printf("No usable resolution.\n");
        return -1;
    }

    printf("Using resolution %dx%d\n", resolution.width, resolution.height);

    videoConfiguration config = {(u8)resolutionId,
                                 VIDEO_BUFFER_FORMAT_XRGB,
                                 VIDEO_ASPECT_AUTO,
                                 {0, 0, 0, 0, 0, 0, 0, 0, 0},
                                 (u32)resolution.width * 4};

    if (videoConfigure(VIDEO_PRIMARY, &config, NULL, 0)) {
        printf("Failed to set resolution\n");
        return -1;
    }

    videoState state;
    videoGetState(VIDEO_PRIMARY, 0, &state);
    switch (state.displayMode.aspect) {
        case VIDEO_ASPECT_4_3:
            displayAspectRatio = 4.0f / 3.0f;
            break;
        default:
            printf("Unknown aspect ratio, assuming 16:9\n");
        case VIDEO_ASPECT_16_9:
            displayAspectRatio = 16.0f / 9.0f;
            break;
    }

    displayWidth = resolution.width;
    displayHeight = resolution.height;

    return 0;
}

void Graphics::waitIdle() {
    const uint8_t labelIndex = 255;
    rsxSetWriteBackendLabel(context, labelIndex, nextLabelValue);
    rsxSetWaitLabel(context, labelIndex, nextLabelValue);
    ++nextLabelValue;
    rsxSetWriteBackendLabel(context, labelIndex, nextLabelValue);
    rsxFlushBuffer(context);

    while (*(vu32*)gcmGetLabelAddress(labelIndex) != nextLabelValue) {
        usleep(30);
    }

    ++nextLabelValue;
}

void Graphics::setRenderTarget(u32 framebufferIndex) {
    gcmSurface surface = {0};

    surface.type = GCM_SURFACE_TYPE_LINEAR;
    surface.colorFormat = GCM_SURFACE_X8R8G8B8;
    surface.colorTarget = GCM_SURFACE_TARGET_0;
    surface.colorOffset[0] = colorBufferOffsets[framebufferIndex];
    surface.colorPitch[0] = colorBufferPitch;
    surface.width = displayWidth;
    surface.height = displayHeight;

    // we don't actually use the other render targets or the depth buffer, but
    // leaving these values at 0 breaks things
    surface.colorPitch[1] = 64;
    surface.colorPitch[2] = 64;
    surface.colorPitch[3] = 64;
    surface.depthFormat = GCM_SURFACE_ZETA_Z24S8;
    surface.depthPitch = 64;

    rsxSetSurface(context, &surface);
}

int Graphics::init() {
    ioBuffer = memalign(ioBufferAlignment, ioBufferSize);
    rsxInit(&context, commandBufferSize, ioBufferSize, ioBuffer);

    if (initVideo()) {
        return -1;
    }

    waitIdle();
    gcmSetFlipMode(GCM_FLIP_VSYNC);

    colorBufferPitch = displayWidth * 4;
    for (uint32_t i = 0; i < framebufferCount; i++) {
        colorBuffers[i] = rsxMemalign(64, displayHeight * colorBufferPitch);
        rsxAddressToOffset(colorBuffers[i], &colorBufferOffsets[i]);
        gcmSetDisplayBuffer(i, colorBufferOffsets[i], colorBufferPitch,
                            displayWidth, displayHeight);
    }

    return 0;
}

void Graphics::beginFrame() {
    setRenderTarget(currentFrameBuffer);

    rsxSetColorMask(context, GCM_COLOR_MASK_B | GCM_COLOR_MASK_G |
                                 GCM_COLOR_MASK_R | GCM_COLOR_MASK_A);
    rsxSetColorMaskMrt(context, 0);

    float min = 0.0f;
    float max = 1.0f;
    float scale[] = {displayWidth * 0.5f, displayHeight * -0.5f,
                     (max - min) * 0.5f, 0.0f};
    float offset[] = {displayWidth * 0.5f, displayHeight * 0.5f,
                      (max + min) * 0.5f, 0.0f};

    rsxSetViewport(context,
                   0,  // x
                   0,  // y
                   displayWidth, displayHeight, min, max, scale, offset);
    rsxSetScissor(context, 0, 0, displayWidth, displayHeight);
    for (int i = 0; i < 8; i++) {
        rsxSetViewportClip(context, i, displayWidth, displayHeight);
    }

    rsxSetDepthTestEnable(context, GCM_FALSE);
    rsxSetDepthWriteEnable(context, GCM_FALSE);
    rsxSetShadeModel(context, GCM_SHADE_MODEL_SMOOTH);
    rsxSetFrontFace(context, GCM_FRONTFACE_CW);

    rsxSetZMinMaxControl(context, 0, 1, 1);

    rsxSetUserClipPlaneControl(
        context, GCM_USER_CLIP_PLANE_DISABLE, GCM_USER_CLIP_PLANE_DISABLE,
        GCM_USER_CLIP_PLANE_DISABLE, GCM_USER_CLIP_PLANE_DISABLE,
        GCM_USER_CLIP_PLANE_DISABLE, GCM_USER_CLIP_PLANE_DISABLE);

    rsxTextureControl(context, 0, GCM_TRUE, 0 << 8, 12 << 8,
                      GCM_TEXTURE_MAX_ANISO_1);
    rsxTextureFilter(context, 0, 0, GCM_TEXTURE_LINEAR, GCM_TEXTURE_LINEAR,
                     GCM_TEXTURE_CONVOLUTION_QUINCUNX);
    rsxTextureWrapMode(context, 0, GCM_TEXTURE_REPEAT, GCM_TEXTURE_REPEAT,
                       GCM_TEXTURE_REPEAT, 0, GCM_TEXTURE_ZFUNC_LESS, 0);

    rsxSetBlendEquation(context, GCM_FUNC_ADD, GCM_FUNC_ADD);
    rsxSetBlendEnable(context, GCM_TRUE);
}

void Graphics::endFrame() {
    if (!firstFrame) {
        while (gcmGetFlipStatus() != 0)
            usleep(200);
    }
    gcmResetFlipStatus();

    gcmSetFlip(context, currentFrameBuffer);
    rsxFlushBuffer(context);
    gcmSetWaitFlip(context);
    currentFrameBuffer = (currentFrameBuffer + 1) % framebufferCount;
    firstFrame = false;
}

void Graphics::finish() {
    gcmSetWaitFlip(context);
    rsxFinish(context, 1);
}
