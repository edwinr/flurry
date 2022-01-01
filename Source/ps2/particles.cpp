#include "particles.h"
#include "convert.h"

#include "../core/Texture.h"

// PS2SDK includes
#include <packet.h>
#include <gs_psm.h>
#include <dma.h>
#include <graph.h>
#include <draw.h>

void FlurryRenderer::loadTexture() {
    const int width = 256;
    const int height = 256;

    auto tempTexData1 = new unsigned char[width * height * 2];
    MakeTexture((unsigned char(*)[width][2])tempTexData1);
    auto tempTexData2 = new unsigned char[width * height * 4];
    for (int i=0; i<width*height; ++i) {
        auto s = &tempTexData1[i * 2];
        auto d = &tempTexData2[i * 4];
        d[0] = d[1] = d[2] = s[0];
        d[3] = s[1];
    }
    delete[] tempTexData1;

    texbuf.width = width;
    texbuf.psm = GS_PSM_32;
    texbuf.info.width = draw_log2(width);
    texbuf.info.height = draw_log2(height);
    texbuf.info.components = TEXTURE_COMPONENTS_RGBA;
    texbuf.info.function = TEXTURE_FUNCTION_MODULATE;
    texbuf.address =
        graph_vram_allocate(width, height, GS_PSM_32, GRAPH_ALIGN_BLOCK);

    auto packet = packet_init(50, PACKET_NORMAL);
    auto q = packet->data;
    q = draw_texture_transfer(q, tempTexData2, width, height, GS_PSM_32,
                                texbuf.address, texbuf.width);
    q = draw_texture_flush(q);
    dma_channel_send_chain(DMA_CHANNEL_GIF, packet->data, q - packet->data,
                            0, 0);
    dma_wait_fast();

    delete[] tempTexData2;
    packet_free(packet);
}

qword_t* FlurryRenderer::bindTexture(qword_t* q) {
    lod_t lod = {0};
    lod.calculation = LOD_USE_K;
    lod.mag_filter = LOD_MAG_LINEAR;
    lod.min_filter = LOD_MIN_LINEAR;
    lod.max_level = 0;
    lod.l = 0;
    lod.k = 0;
    q = draw_texture_sampling(q, 0, &lod);

    clutbuffer_t clut = {0};
    q = draw_texturebuffer(q, 0, &texbuf, &clut);

    texwrap_t wrap = {0};
    wrap.horizontal = WRAP_REPEAT;
    wrap.vertical = WRAP_REPEAT;
    q = draw_texture_wrapping(q, 0, &wrap);
    return q;
}

qword_t* FlurryRenderer::setFlurryBlendMode(qword_t* q) {
    // this blend equation is not like on PC:
    // (color1 - color2) * alpha + color3
    blend_t blend;
    blend.color1 = BLEND_COLOR_SOURCE;
    blend.color2 = BLEND_COLOR_ZERO;
    blend.alpha  = BLEND_ALPHA_SOURCE;
    blend.color3 = BLEND_COLOR_DEST;
    blend.fixed_alpha = 0x80;
    q = draw_alpha_blending(q, 0, &blend);

    return q;
}

qword_t* FlurryRenderer::render(qword_t* q, global_info_t& flurryInfo, MATRIX projectionMatrix) {
    q = setFlurryBlendMode(q);
    q = bindTexture(q);

    prim_t prim;
    prim.type = PRIM_TRIANGLE;
    prim.shading = PRIM_SHADE_GOURAUD;
    prim.mapping = DRAW_ENABLE;
    prim.fogging = DRAW_DISABLE;
    prim.blending = DRAW_ENABLE;
    prim.antialiasing = DRAW_DISABLE;
    prim.mapping_type = PRIM_MAP_ST;
    prim.colorfix = PRIM_UNFIXED;

    color_t color;
    color.r = 0xff;
    color.g = 0xff;
    color.b = 0xff;
    color.a = 0xff;
    color.q = 1.0f;
    q = draw_prim_start(q, 0, &prim, &color);
    auto dw = (u64*)q;
    
    for (int i = 0; i < flurryInfo.s->numQuads; i++) {
        auto quadPositions = &((float*)&flurryInfo.s->seraphimVertices)[i * 8];
        VECTOR transformedPositions[4];
        xyz_t convertedPositions[4];
        for (int j = 0; j< 4; j++) {
            VECTOR position = {quadPositions[j * 2], quadPositions[j * 2 + 1], 0.0f, 1.0f};
            vector_apply(transformedPositions[j], position,
                            projectionMatrix);
            convertedPositions[j] = convertXyz(transformedPositions[j]);
        }

        color_t convertedColors[4];
        draw_convert_rgbaq(convertedColors, 4, (vertex_f_t*)&transformedPositions, (color_f_t*)&((float*)&flurryInfo.s->seraphimColors)[i * 4 * 4]);
        
        VECTOR texCoords[] = {{flurryInfo.s->seraphimTextures[i * 8 + 0], flurryInfo.s->seraphimTextures[i * 8 + 1], 0, 0},
                                {flurryInfo.s->seraphimTextures[i * 8 + 2], flurryInfo.s->seraphimTextures[i * 8 + 3], 0, 0},
                                {flurryInfo.s->seraphimTextures[i * 8 + 4], flurryInfo.s->seraphimTextures[i * 8 + 5], 0, 0},
                                {flurryInfo.s->seraphimTextures[i * 8 + 6], flurryInfo.s->seraphimTextures[i * 8 + 7], 0, 0}};
        texel_t convertedTexCoords[4];
        draw_convert_st(convertedTexCoords, 4, (vertex_f_t*)&transformedPositions, (texel_f_t*)texCoords);


        const int indices[] = {0, 2, 1, 0, 3, 2};
        for (int j=0; j<6; ++j) {
            auto index = indices[j];
            *dw++ = convertedColors[index].rgbaq;
            *dw++ = convertedTexCoords[index].uv;
            *dw++ = convertedPositions[index].xyz;
        }
    }

    if ((u32)dw % 16) {
        *dw++ = 0;
    }

    q = (qword_t*)dw;
    q = draw_prim_end(q, 3, DRAW_STQ_REGLIST);
    return q;
}