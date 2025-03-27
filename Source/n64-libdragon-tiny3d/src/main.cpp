#include "../../core/Default.h"
#include "../../core/Gl_saver.h"
#include "../../core/Texture.h"

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3ddebug.h>

#include <memory>

static void darkenBackground(float alpha, int32_t width, int32_t height);
static void drawParticles(int numPrimitives,
                          float* positionPtr,
                          float* colorPtr,
                          float* texcoordPtr);
static void prepareQuadIndices();
static void prepareTexture();

static T3DVertPacked* packedVertices;
static const int quadsPerBatch = 16;
static int16_t* quadIndices;
static surface_t texSurface;

int main(void) {
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE,
                 FILTERS_RESAMPLE);
    rdpq_init();

    t3d_init((T3DInitParams){});

    T3DMat4 modelMat;
    T3DMat4FP* modelMatFP = (T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4_identity(&modelMat);
    t3d_mat4_scale(&modelMat, 0.1f, 0.1f, 0.1f);
    t3d_mat4_to_fixed(modelMatFP, &modelMat);

    packedVertices = (T3DVertPacked*)malloc_uncached(
        sizeof(T3DVertPacked) * NUMSMOKEPARTICLES * 4 + 16);
    prepareQuadIndices();

    rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO,
                            rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO));

    auto info = std::make_unique<global_info_t>();
    int width = display_get_width(), height = display_get_height();
    initDefaultFlurry(info.get(), width, height);
    GLSetupRC(info.get());
    GLResize(info.get(), width, height);

    T3DViewport viewport = t3d_viewport_create();
    T3DMat4 viewMatrix;
    t3d_mat4_identity(&viewMatrix);
    t3d_viewport_set_ortho(&viewport, 0, width, 0, height, -1000.0f, 100.1f);
    t3d_viewport_set_view_matrix(&viewport, &viewMatrix);

    prepareTexture();

    double time = 0.0;
    double deltaTime = 1.0 / 30.0;
    while (true) {
        time += deltaTime;
        double alpha = 5.0 * deltaTime;
        if (alpha > 0.2)
            alpha = 0.2;

        GLRenderScene(info.get(), time, deltaTime);

        rdpq_attach(display_get(), nullptr);

        t3d_frame_start();
        rdpq_mode_begin();
        rdpq_mode_zbuf(false, false);
        rdpq_mode_antialias(AA_NONE);
        rdpq_mode_end();

        t3d_viewport_attach(&viewport);
        t3d_matrix_push(modelMatFP);

        t3d_light_set_ambient({0xff, 0xff, 0xff, 0xff});
        t3d_state_set_drawflags(T3D_FLAG_SHADED);

        darkenBackground(alpha, width, height);
        t3d_state_set_drawflags(
            (T3DDrawFlags)(T3D_FLAG_SHADED | T3D_FLAG_TEXTURED));
        drawParticles(info->s->numQuads * 4, (float*)info->s->seraphimVertices,
                      (float*)info->s->seraphimColors,
                      (float*)info->s->seraphimTextures);
        t3d_matrix_pop(1);

        rdpq_sync_pipe();
        rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 40, 220, "FPS   : %.2f",
                         display_get_fps());
        rdpq_detach_show();
    }
}

void darkenBackground(float alpha, int32_t width, int32_t height) {
    rdpq_mode_begin();
    rdpq_mode_dithering(DITHER_NONE_NONE);
    rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_mode_end();

    rdpq_set_prim_color(RGBA32(0, 0, 0, (uint8_t)(alpha * 255.0)));
    rdpq_fill_rectangle(0, 0, width, height);
}

static float saturate(float f) {
    if (f < 0.0f) {
        return 0.0f;
    } else if (f > 1.0) {
        return 1.0f;
    }
    return f;
}

void preparePackedVertices(int numPrimitives,
                           float* positionPtr,
                           float* colorPtr,
                           float* texcoordPtr) {
    T3DVec3 normVec = {0, 0, 1};
    uint16_t norm = t3d_vert_pack_normal(&normVec);

    const float scale = 10.0f;
    auto pos = positionPtr;
    auto col = colorPtr;
    auto tex = texcoordPtr;
    auto packedVertex = packedVertices;
    for (int i = 0; i < numPrimitives; i += 4) {
        uint32_t color = (uint32_t)(saturate(col[0]) * 255.0f) << 24 |
                         (uint32_t)(saturate(col[1]) * 255.0f) << 16 |
                         (uint32_t)(saturate(col[2]) * 255.0f) << 8 |
                         (uint32_t)(saturate(col[3]) * 255.0f);

        for (int j = 0; j < 2; j++) {
            packedVertex->posA[0] = pos[0] * scale;
            packedVertex->posA[1] = pos[1] * scale;
            packedVertex->posA[2] = 0;
            packedVertex->posB[0] = pos[2] * scale;
            packedVertex->posB[1] = pos[3] * scale;
            packedVertex->posB[2] = 0;

            packedVertex->normA = norm;
            packedVertex->normB = norm;

            packedVertex->rgbaA = color;
            packedVertex->rgbaB = color;

            packedVertex->stA[0] = (int16_t)(tex[0] * 32.0f * 32.0f);
            packedVertex->stA[1] = (int16_t)(tex[1] * 32.0f * 32.0f);
            packedVertex->stB[0] = (int16_t)(tex[2] * 32.0f * 32.0f);
            packedVertex->stB[1] = (int16_t)(tex[3] * 32.0f * 32.0f);

            pos += 4;
            col += 8;
            tex += 4;
            ++packedVertex;
        }
    }
}

void drawParticles(int numPrimitives,
                   float* positionPtr,
                   float* colorPtr,
                   float* texcoordPtr) {
    preparePackedVertices(numPrimitives, positionPtr, colorPtr, texcoordPtr);

    rdpq_mode_begin();
    rdpq_mode_dithering(DITHER_NOISE_NOISE);
    rdpq_mode_combiner(RDPQ_COMBINER_TEX_SHADE);
    rdpq_mode_blender(RDPQ_BLENDER_ADDITIVE);
    rdpq_mode_end();

    rdpq_sync_tile();
    rdpq_mode_tlut(TLUT_NONE);
    rdpq_tex_upload(TILE0, &texSurface, NULL);

    auto completeBatches = numPrimitives / (quadsPerBatch * 4);
    for (int i = 0; i < completeBatches; ++i) {
        t3d_vert_load(&packedVertices[i * quadsPerBatch * 2], 0,
                      quadsPerBatch * 4);
        t3d_tri_draw_strip(quadIndices, quadsPerBatch * 4);
        t3d_tri_sync();
    }

    auto remaining = numPrimitives - completeBatches * quadsPerBatch * 4;
    if (remaining <= 0) {
        return;
    }

    auto offset = completeBatches * quadsPerBatch * 4;
    t3d_vert_load(&packedVertices[offset], 0, remaining);
    for (int i = 0; i < remaining; i += 4) {
        t3d_tri_draw(i + 0, i + 1, i + 2);
        t3d_tri_draw(i + 2, i + 3, i + 0);
    }
    t3d_tri_sync();
}

void prepareQuadIndices() {
    quadIndices =
        (int16_t*)malloc_uncached(sizeof(int16_t) * quadsPerBatch * 4);

    auto ptr = quadIndices;
    for (int i = 0; i < quadsPerBatch; ++i) {
        auto offset = i * 4;
        *ptr++ = (offset + 0) | (1 << 15);
        *ptr++ = (offset + 1);
        *ptr++ = (offset + 3);
        *ptr++ = (offset + 2);
    }

    t3d_indexbuffer_convert(quadIndices, quadsPerBatch * 4);
}

void prepareTexture() {
    auto bigTextureArray = new unsigned char[256 * 256 * 2];
    MakeTexture((unsigned char(*)[256][2])bigTextureArray);

    texSurface = surface_alloc(FMT_IA8, 32, 32);
    auto ptrI = (uint8_t*)texSurface.buffer;
    auto ptrA = ((uint8_t*)texSurface.buffer) + 32 * 32;
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            int index = (y * 256 + x) * 2;
            *ptrI++ = bigTextureArray[index];
            *ptrA++ = bigTextureArray[index + 1];
        }
    }

    delete[] bigTextureArray;
}
