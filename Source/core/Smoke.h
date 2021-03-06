// Smoke.h: interface for the Smoke class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SMOKE_H)
#define SMOKE_H

#include "SmokeParticle.h"
#include "Star.h"
#include "Spark.h"

#define NUMSMOKEPARTICLES 3600

typedef struct SmokeV {
    SmokeParticleV p[NUMSMOKEPARTICLES / 4];
    int nextParticle;
    int nextSubParticle;
    float lastParticleTime;
    int firstTime;
    long frame;
    float old[3];

    int numQuads;
    floatToVector seraphimVertices[NUMSMOKEPARTICLES * 2 + 1];
    floatToVector seraphimColors[NUMSMOKEPARTICLES * 4 + 1];
    float seraphimTextures[NUMSMOKEPARTICLES * 2 * 4];
} SmokeV;

void InitSmoke(SmokeV* s);

void UpdateSmoke_ScalarBase(SmokeV* s,
                            Star* star,
                            double fTime,
                            double fDeltaTime,
                            int numStreams,
                            Spark** sparks,
                            int dframe,
                            float drag);

void PrepareSmokeVertices_Scalar(SmokeV* s,
                      float screenWidth,
                      float screenHeight,
                      float streamExpansion,
                      double fTime);

#endif  // !defined(SMOKE_H)
