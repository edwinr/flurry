#include <chrono>

#include "Gl_saver.h"
#include "Std.h"
#include "Smoke.h"
#include "Star.h"
#include "Spark.h"
#include "Particle.h"
#include <math.h>

double TimeInSecondsSinceStart(void) {
    static auto start = std::chrono::high_resolution_clock::now();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    return diff.count();
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// Do any initialization of the rendering context here, such as
// setting background colors, setting up lighting, or performing
// preliminary calculations.
void GLSetupRC(global_info_t* info) {
    int i, k;

    info->spark[0]->mystery = 1800 / 13;
    info->spark[1]->mystery = (1800 * 2) / 13;
    info->spark[2]->mystery = (1800 * 3) / 13;
    info->spark[3]->mystery = (1800 * 4) / 13;
    info->spark[4]->mystery = (1800 * 5) / 13;
    info->spark[5]->mystery = (1800 * 6) / 13;
    info->spark[6]->mystery = (1800 * 7) / 13;
    info->spark[7]->mystery = (1800 * 8) / 13;
    info->spark[8]->mystery = (1800 * 9) / 13;
    info->spark[9]->mystery = (1800 * 10) / 13;
    info->spark[10]->mystery = (1800 * 11) / 13;
    info->spark[11]->mystery = (1800 * 12) / 13;

    for (i = 0; i < NUMSMOKEPARTICLES / 4; i++) {
        for (k = 0; k < 4; k++) {
            info->s->p[i].dead.i[k] = 1;
        }
    }

    for (i = 0; i < 12; i++) {
        UpdateSpark(info->spark[i], info->fTime, info->fDeltaTime, info->currentColorMode, info->flurryRandomSeed);
    }

    info->fOldTime = TimeInSecondsSinceStart() + info->flurryRandomSeed;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// Render the OpenGL Scene here. Called by the WM_PAINT message
// handler.
void GLRenderScene(global_info_t* info) {
    int i;

    info->dframe++;

    info->fOldTime = info->fTime;
    info->fTime = TimeInSecondsSinceStart() + info->flurryRandomSeed;
    info->fDeltaTime = info->fTime - info->fOldTime;

    info->drag = (float)pow(0.9965, info->fDeltaTime * 85.0);

    for (i = 0; i < numParticles; i++) {
        UpdateParticle(info->p[i], info->fDeltaTime);
    }
    UpdateStar(info->star, info->fTime);
    for (i = 0; i < info->numStreams; i++) {
        UpdateSpark(info->spark[i], info->fTime, info->fDeltaTime, info->currentColorMode, info->flurryRandomSeed);
    }

    UpdateSmoke_ScalarBase(info->s, info->star, info->fTime, info->fDeltaTime,
                           info->numStreams, info->spark, info->dframe,
                           info->drag);

    PrepareSmokeVertices_Scalar(info->s, info->sys_glWidth, info->sys_glHeight, info->streamExpansion, info->fTime);
}

void GLResize(global_info_t* info, float w, float h) {
    info->sys_glWidth = w;
    info->sys_glHeight = h;
}
