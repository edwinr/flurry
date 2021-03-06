// Particle.h: interface for the Particle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(PARTICLE_H)
#define PARTICLE_H

typedef struct Particle {
    float charge;
    float x;
    float y;
    float z;
    float oldx;
    float oldy;
    float oldz;
    float deltax;
    float deltay;
    float deltaz;
    float r;
    float g;
    float b;
    int animFrame;
} Particle;

void InitParticle(Particle* p, float screenWidth, float screenHeight);
void UpdateParticle(Particle* p, double fDeltaTime);

#endif  // !defined(PARTICLE_H)
