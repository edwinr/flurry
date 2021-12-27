#include "../core/Gl_saver.h"

#include <memory>
#include <cstdlib>

#include <packet.h>
#include <dma_tags.h>
#include <gs_psm.h>
#include <dma.h>
#include <graph.h>
#include <draw.h>
#include <draw3d.h>

const int screenWidth = 640;
const int screenHeight = 512;
const float screenScaleX = 1.25f;
const float screenScaleY = 1.0f;

static xyz_t convertXyz(VECTOR v) {
    const unsigned int maxZ = 1 << 31;
    
    xyz_t result;
    
    const float factorX = 4096.0f * screenScaleX;
    const float factorY = -4096.0f * screenScaleY;
    result.x = (short)((v[0] - 1.0f) * factorX + 32768.0f);
    result.y = (short)((v[1] - 1.0f) * factorY + 32768.0f);
    result.z = (unsigned int)(v[2] * maxZ);
    return result;
}

struct FlurryRenderData {
    VECTOR sourcePositions[NUMSMOKEPARTICLES * 4];
    VECTOR transformedPositions[NUMSMOKEPARTICLES * 4];
    xyz_t convertedPositions[NUMSMOKEPARTICLES * 4];
    float* sourceColors;
    int numVertices;

    qword_t* setFlurryBlendMode(qword_t* q) {
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

    void prepareVertexData(global_info_t* info, MATRIX projectionMatrix) {
        //auto sourceVertices = &(reinterpret_cast<float*>(&info->s->seraphimVertices))[i * 2 * 4];
        //auto sourceTexCoords = &info->s->seraphimTextures[sourceOffset];
        sourceColors = (float*)&info->s->seraphimColors;
        
        numVertices = info->s->numQuads * 4;
        if (numVertices > 0) {
            auto s = (float*)&info->s->seraphimVertices;

            for (int i = 0; i < numVertices; i++) {
                sourcePositions[i][0] = s[0];
                sourcePositions[i][1] = s[1];
                sourcePositions[i][2] = 0.0f;
                sourcePositions[i][3] = 0.0f;

                vector_apply(transformedPositions[i], sourcePositions[i],
                             projectionMatrix);
                s += 2;
            }
            
            for (int i = 0; i < numVertices; ++i) {
                convertedPositions[i] = convertXyz(transformedPositions[i]);
            }
        }
    }

    qword_t* render(qword_t* q) {
        q = setFlurryBlendMode(q);

        prim_t prim;
        prim.type = PRIM_TRIANGLE;
        prim.shading = PRIM_SHADE_GOURAUD;
        prim.mapping = DRAW_DISABLE;
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
        for (int i = 0; i < numVertices / 4; i++) {
            color_t convertedColors[4];
            draw_convert_rgbaq(convertedColors, 4, (vertex_f_t*)&transformedPositions[i * 4], (color_f_t*)&sourceColors[i * 4 * 4]);

            const int indices[] = {0, 1, 2, 0, 2, 3};
            for (int j=0; j<6; ++j) {
                q->dw[0] = convertedColors[indices[j]].rgbaq;
                q->dw[1] = convertedPositions[i * 4 + indices[j]].xyz;
                q++;
            }
        }

        q = draw_prim_end(q, 2, DRAW_RGBAQ_REGLIST);
        return q;
    }
};

static qword_t* darkenBackground(qword_t* q) {
    const float alpha = 0.083333333335f;
    // VECTOR colors[] = {{1.0f, 0.0f, 0.0f, alpha},
    //                    {0.0f, 1.0f, 0.0f, alpha},
    //                    {0.0f, 0.0f, 1.0f, alpha},
    //                    {0.0f, 1.0f, 1.0f, alpha}};
    VECTOR colors[] = {{0.0f, 0.0f, 0.0f, alpha},
                       {0.0f, 0.0f, 0.0f, alpha},
                       {0.0f, 0.0f, 0.0f, alpha},
                       {0.0f, 0.0f, 0.0f, alpha}};
    VECTOR positions[] = {{0, 0, 0, 1},
                          {2, 0, 0, 1},
                          {2, 2, 0, 1},
                          {0, 0, 0, 1}};
    xyz_t convertedPositions[4];
    color_t convertedColors[4];

    for (int i=0; i<3; ++i) {
        convertedPositions[i] = convertXyz(positions[i]);
    }
    draw_convert_rgbaq(convertedColors, 3, (vertex_f_t*)positions, (color_f_t*)colors);

    blend_t blend;
    blend.color1 = BLEND_COLOR_SOURCE;
    blend.color2 = BLEND_COLOR_DEST;
    blend.alpha  = BLEND_ALPHA_SOURCE;
    blend.color3 = BLEND_COLOR_DEST;
    blend.fixed_alpha = 0x80;
    q = draw_alpha_blending(q, 0, &blend);

    prim_t prim;
    prim.type = PRIM_TRIANGLE_FAN;
    prim.shading = PRIM_SHADE_GOURAUD;
    prim.mapping = DRAW_DISABLE;
    prim.fogging = DRAW_DISABLE;
    prim.blending = DRAW_ENABLE;
    prim.antialiasing = DRAW_DISABLE;
    prim.mapping_type = PRIM_MAP_ST;
    prim.colorfix = PRIM_UNFIXED;

    color_t color = {0};
    q = draw_prim_start(q, 0, &prim, &color);
    for (int i=0; i<4; ++i) {
        q->dw[0] = convertedColors[i].rgbaq;
        q->dw[1] = convertedPositions[i].xyz;
        q++;
    }
    q = draw_prim_end(q, 2, DRAW_RGBAQ_REGLIST);

    return q;
}

int renderLoop(global_info_t& flurryInfo, FlurryRenderData& renderData) {
    // glOrtho(0, screenWidth, 0, screenHeight, 0.0, 1.0);
    float left = 0, right = screenWidth, bottom = 0, top = screenHeight,
          zNear = 0, zFar = 1;

    MATRIX projectionMatrix;
    matrix_unit(projectionMatrix);
    projectionMatrix[0] = 2.0f / (right - left);
    projectionMatrix[5] = 2.0f / (top - bottom);
    projectionMatrix[10] = -2.0f / (zFar - zNear);
    projectionMatrix[3] = -(right + left) / (right - left);
    projectionMatrix[7] = -(top + bottom) / (top - bottom);
    projectionMatrix[11] = -(zFar + zNear) / (zFar - zNear);

    packet_t* packets[2];
    packet_t* current;

    packets[0] = packet_init(NUMSMOKEPARTICLES * 6 + 100, PACKET_NORMAL);
    packets[1] = packet_init(NUMSMOKEPARTICLES * 6 + 100, PACKET_NORMAL);
    dma_wait_fast();

    int context = 0;
    for (;;) {
        GLRenderScene(&flurryInfo);
        renderData.prepareVertexData(&flurryInfo, projectionMatrix);

        current = packets[context];
        
        qword_t* dmatag = current->data;
        qword_t* q = dmatag;
        q++;

        q = darkenBackground(q);
        q = renderData.render(q);
        q = draw_finish(q);

        DMATAG_END(dmatag, (q - current->data) - 1, 0, 0, 0);

        dma_wait_fast();
        dma_channel_send_chain(DMA_CHANNEL_GIF, current->data,
                               q - current->data, 0, 0);

        draw_wait_finish();
        graph_wait_vsync();

        context ^= 1;
    }

    packet_free(packets[0]);
	packet_free(packets[1]);

	return 0;
}

static void initFlurry(global_info_t* flurry_info) {
    flurry_info->flurryRandomSeed = RandFlt(0.0, 300.0);

    flurry_info->numStreams = 5;
    flurry_info->streamExpansion = 100;
    flurry_info->currentColorMode = tiedyeColorMode;

    for (int i = 0; i < MAXNUMPARTICLES; i++) {
        flurry_info->p[i] = new Particle;
    }

    flurry_info->s = new SmokeV;
    InitSmoke(flurry_info->s);

    flurry_info->star = new Star;
    InitStar(flurry_info->star);
    flurry_info->star->rotSpeed = 1.0;

    for (int i = 0; i < 64; i++) {
        flurry_info->spark[i] = new Spark;
        InitSpark(flurry_info->spark[i]);
    }
}

int main(int argc, char** argv) {
    auto flurryInfo = std::make_unique<global_info_t>();
    auto renderData = std::make_unique<FlurryRenderData>();

    initFlurry(flurryInfo.get());
    GLSetupRC(flurryInfo.get());
    GLResize(flurryInfo.get(), screenWidth, screenHeight);


	dma_channel_initialize(DMA_CHANNEL_GIF,NULL,0);
	dma_channel_fast_waits(DMA_CHANNEL_GIF);

	framebuffer_t frame = {0};
    frame.width = screenWidth;
    frame.height = screenHeight;
    frame.psm = GS_PSM_32;
    frame.address = graph_vram_allocate(frame.width, frame.height, frame.psm,
                                        GRAPH_ALIGN_PAGE);

    // No zbuffer
	zbuffer_t z = {0};

    graph_initialize(frame.address, frame.width, frame.height, frame.psm, 0, 0);

    packet_t* packet = packet_init(16, PACKET_NORMAL);
    qword_t* q = packet->data;
    q = draw_setup_environment(q, 0, &frame, &z);
    q = draw_primitive_xyoffset(q, 0, (2048 - screenWidth / 2),
                                (2048 - screenHeight / 2));
    q = draw_finish(q);
    dma_channel_send_normal(DMA_CHANNEL_GIF, packet->data, q - packet->data, 0,
                            0);
    dma_wait_fast();
    packet_free(packet);

    renderLoop(*flurryInfo, *renderData);

    return 0;
}
