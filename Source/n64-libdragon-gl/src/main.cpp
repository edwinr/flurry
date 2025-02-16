#include "../../core/Gl_saver.h"
#include "../../core/Texture.h"
#include "../../opengl/FlurryOpenGL.h"

#include <libdragon.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/gl_integration.h>

#include <memory>

static void initFlurry(global_info_t* flurry_info, int width, int height) {
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

    for (int i = 0; i < numParticles; i++) {
        InitParticle(flurry_info->p[i], width, height);
    }
}

int main(void)
{
    display_init(RESOLUTION_320x240, DEPTH_32_BPP, 3, GAMMA_NONE,
                 FILTERS_RESAMPLE_ANTIALIAS);
    rdpq_init();
    gl_init();
    
    auto info = std::make_unique<global_info_t>();
    int width = display_get_width(), height = display_get_height();
    initFlurry(info.get(), width, height);
    GLSetupRC(info.get());

	rdpq_attach_clear(display_get(), nullptr);
	rdpq_set_mode_fill(RGBA32(0x00, 0xFF, 0xFF, 0));
    rdpq_fill_rectangle(0, 0, width, height);

	gl_context_begin();
    glDisable(GL_DITHER);
    FlurryOpenGL flurryOpenGL;
    flurryOpenGL.init(width, height);
    GLResize(info.get(), width, height);
	gl_context_end();
    
    rdpq_detach_show();
	double time = 0.0;
	double deltaTime = 1.0 / 30.0;
    while(true) {
        rdpq_attach(display_get(), nullptr);
		
		time += deltaTime;
        double alpha = 5.0 * deltaTime;
        if (alpha > 0.2)
            alpha = 0.2;

        gl_context_begin();
        flurryOpenGL.darkenBackground(alpha);
        GLRenderScene(info.get(), time, deltaTime);
        flurryOpenGL.drawFlurryParticles(
            info->s->numQuads * 4, info->s->seraphimVertices,
            info->s->seraphimColors, info->s->seraphimTextures);
		gl_context_end();

        rdpq_detach_show();
    }
}
