// Particle.c: implementation of the Particle class.
//
//////////////////////////////////////////////////////////////////////

#include "Std.h"
#include "Particle.h"
#include <cstdlib>
#define MAXANGLES 16384
extern int theTexture;

void UpdateParticle(Particle* p, double fDeltaTime) {
    p->oldx = p->x;
    p->oldy = p->y;
    p->oldz = p->z;

    p->x += p->deltax * fDeltaTime;
    p->y += p->deltay * fDeltaTime;
    p->z += p->deltaz * fDeltaTime;
}

void InitParticle(Particle* p, float screenWidth, float screenHeight) {
    //	float tempx,tempy;
    int r1, r2;
    p->oldz = RandFlt(2500.0f, 22500.0f);
    //	do
    //	{
    r1 = rand();
    r2 = rand();
    p->oldx =
        ((float)(r1 % (int)screenWidth) - screenWidth * 0.5f) /
        (screenWidth / p->oldz);
    p->oldy =
        (screenHeight * 0.5f - (float)(r2 % (int)screenHeight)) /
        (screenWidth / p->oldz);
    //		tempx = (oldx * screenWidth / 75.0f) +
    // screenWidth/2.0f;
    //		tempy = (oldy * screenWidth / 75.0f) +
    // screenHeight/2.0f;
    //	} while (fabs(tempx) < screenWidth + 100.0 && fabs(tempy) <
    // screenHeight + 100.0);
    p->deltax = 0.0f;
    p->deltay = 0.0f;
    p->deltaz = (float)-starSpeed;
    p->x = p->oldx + p->deltax;
    p->y = p->oldy + p->deltay;
    p->z = p->oldz + p->deltaz;
    p->r = RandFlt(0.125f, 1.0f);
    p->g = RandFlt(0.125f, 1.0f);
    p->b = RandFlt(0.125f, 1.0f);
    p->animFrame = 0;
}
