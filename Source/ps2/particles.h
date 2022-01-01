#ifndef Flurry_ps2_particles_h
#define Flurry_ps2_particles_h
#include <tamtypes.h>
#include <draw_buffers.h>
#include <math3d.h>
#include "../core/Gl_saver.h"

struct FlurryRenderer {
    texbuffer_t texbuf;

    void loadTexture();

    qword_t* bindTexture(qword_t* q);
    qword_t* setFlurryBlendMode(qword_t* q);
    qword_t* render(qword_t* q, global_info_t& flurryInfo, MATRIX projectionMatrix);
};

#endif
