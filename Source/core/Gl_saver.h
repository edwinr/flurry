#ifndef __GLCODE__
#define __GLCODE__



#include "Smoke.h"
#include "Star.h"
#include "Spark.h"
#include "Particle.h"
#include "Std.h"

typedef enum _ColorModes {
    redColorMode = 0,
    magentaColorMode,
    blueColorMode,
    cyanColorMode,
    greenColorMode,
    yellowColorMode,
    slowCyclicColorMode,
    cyclicColorMode,
    tiedyeColorMode,
    rainbowColorMode,
    whiteColorMode,
    multiColorMode,
    darkColorMode
} ColorModes;

#define MAXNUMPARTICLES 2500

typedef struct global_info_t {
    double flurryRandomSeed;
    double fTime;
    double fOldTime;
    double fDeltaTime;
#define gravity 1500000.0f
    float sys_glWidth;
    float sys_glHeight;
    float drag;
#define MouseX 0
#define MouseY 0
#define MouseDown 0

    ColorModes currentColorMode;
    float streamExpansion;
    int numStreams;

#define incohesion 0.07f
#define colorIncoherence 0.15f
#define streamSpeed 450.0
#define fieldCoherence 0
#define fieldSpeed 12.0f
#define numParticles 250
#define starSpeed 50
#define seraphDistance 2000.0f
#define streamSize 25000.0f
#define fieldRange 1000.0f
#define streamBias 7.0f

    int dframe;
    float starfieldColor[MAXNUMPARTICLES * 4 * 4];
    float starfieldVertices[MAXNUMPARTICLES * 2 * 4];
    float starfieldTextures[MAXNUMPARTICLES * 2 * 4];
    int starfieldColorIndex;
    int starfieldVerticesIndex;
    int starfieldTexturesIndex;
    Particle* p[MAXNUMPARTICLES];
    SmokeV* s;
    Star* star;
    Spark* spark[64];
} global_info_t;

#define kNumSpectrumEntries 512

double TimeInSecondsSinceStart();

void GLSetupRC(global_info_t* info);
void GLRenderScene(global_info_t* info);
void GLRenderScene(global_info_t* info, double time, double deltaTime);
void GLResize(global_info_t* info, float w, float h);

#endif  // Include/Define
