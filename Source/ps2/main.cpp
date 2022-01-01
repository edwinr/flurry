#include "fade.h"
#include "convert.h"
#include "particles.h"

#include "../core/Gl_saver.h"

#include <memory>
#include <cstdlib>

#include <packet.h>
#include <dma_tags.h>
#include <gs_psm.h>
#include <dma.h>
#include <graph.h>
#include <draw.h>

static int renderLoop(global_info_t& flurryInfo, FlurryRenderer& renderData);
static void setProjectionMatrix(MATRIX projectionMatrix);
static void initFlurry(global_info_t* flurry_info);

int main(int argc, char** argv) {
    auto flurryInfo = std::make_unique<global_info_t>();
    auto renderData = std::make_unique<FlurryRenderer>();

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

    renderData->loadTexture();
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

int renderLoop(global_info_t& flurryInfo, FlurryRenderer& renderData) {
    MATRIX projectionMatrix;
    setProjectionMatrix(projectionMatrix);

    packet_t* packets[2];
    packet_t* current;

    packets[0] = packet_init(NUMSMOKEPARTICLES * 6 + 100, PACKET_NORMAL);
    packets[1] = packet_init(NUMSMOKEPARTICLES * 6 + 100, PACKET_NORMAL);
    dma_wait_fast();

    int context = 0;
    double time = flurryInfo.flurryRandomSeed;
    for (;;) {
        GLRenderScene(&flurryInfo, time, 1.0 / 60.0);
        time += 1.0 / 60.0;

        current = packets[context];
        
        qword_t* dmatag = current->data;
        qword_t* q = dmatag;
        q++;

        q = darkenBackground(q);
        q = renderData.render(q, flurryInfo, projectionMatrix);
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

static void setProjectionMatrix(MATRIX projectionMatrix) {
    // glOrtho(0, screenWidth, 0, screenHeight, 0.0, 1.0);
    float left = 0, right = screenWidth, bottom = 0, top = screenHeight,
          zNear = 0, zFar = 1;

    matrix_unit(projectionMatrix);
    projectionMatrix[0] = 2.0f / (right - left);
    projectionMatrix[5] = 2.0f / (top - bottom);
    projectionMatrix[10] = -2.0f / (zFar - zNear);
    projectionMatrix[3] = -(right + left) / (right - left);
    projectionMatrix[7] = -(top + bottom) / (top - bottom);
    projectionMatrix[11] = -(zFar + zNear) / (zFar - zNear);
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
