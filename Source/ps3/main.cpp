#include <io/pad.h>
#include <sys/process.h>
#include <sysutil/sysutil.h>
#include <memory>
#include "../core/Gl_saver.h"
#include "Graphics.h"
#include "particles.h"

SYS_PROCESS_PARAM(1001, 0x100000);

static bool crossIsPressed();
static void handleSysEvent(u64 status, u64 param, void* usrdata);
static void initFlurry(global_info_t* flurry_info);
static void destructFlurry(global_info_t* flurry_info);

int main(int argc, const char* argv[]) {
    ioPadInit(7);

    Graphics graphics;
    if (graphics.init()) {
        printf("Graphics::init failed\n");
    }

    FlurryRenderer particles;
    if (particles.init()) {
        printf("FlurryRenderer::init failed\n");
    }

    auto info = std::make_unique<global_info_t>();
    initFlurry(info.get());
    GLSetupRC(info.get());
    GLResize(info.get(), graphics.displayWidth, graphics.displayHeight);

    bool running = true;
    sysUtilRegisterCallback(0, handleSysEvent, &running);
    while (running) {
        sysUtilCheckCallback();

        if (crossIsPressed()) {
            break;
        }

        graphics.beginFrame();
        GLRenderScene(info.get());  // update the simulation
        particles.draw(graphics.context, info.get(), graphics.displayWidth,
                       graphics.displayHeight);
        graphics.endFrame();
    }

    printf("Shutting down\n");
    graphics.finish();
    destructFlurry(info.get());
    return 0;
}

static bool crossIsPressed() {
    padInfo padinfo = {0};
    ioPadGetInfo(&padinfo);
    for (int i = 0; i < MAX_PADS; i++) {
        if (padinfo.status[i]) {
            padData paddata = {0};
            ioPadGetData(i, &paddata);

            if (paddata.BTN_CROSS) {
                printf("x ingedrukt op pad %d\n", i);
                return true;
            }
        }
    }
    return false;
}

static void handleSysEvent(u64 status, u64 param, void* usrdata) {
    bool* running = (bool*)usrdata;

    switch (status) {
        case SYSUTIL_EXIT_GAME:
            *running = false;
            break;
        case SYSUTIL_DRAW_BEGIN:
        case SYSUTIL_DRAW_END:
            break;
        default:
            break;
    }
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

static void destructFlurry(global_info_t* flurry_info) {
    for (int i = 0; i < MAXNUMPARTICLES; i++) {
        delete flurry_info->p[i];
    }

    delete flurry_info->s;
    delete flurry_info->star;
    for (int i = 0; i < 64; i++) {
        delete flurry_info->spark[i];
    }
}