#ifndef Flurry_ps2_convert_h
#define Flurry_ps2_convert_h

#include <math3d.h>
#include <draw_types.h>

//#define FLURRY_PS2_720P
#ifdef FLURRY_PS2_720P
const int screenWidth = 1280;
const int screenHeight = 720;
#else
const int screenWidth = 640;
const int screenHeight = 512;
#endif
const float screenScaleX = screenWidth / 512.0f;
const float screenScaleY = screenHeight / 512.0f;

inline static xyz_t convertXyz(VECTOR v) {
    const unsigned int maxZ = 1 << 31;
    
    xyz_t result;
    
    const float factorX = 4096.0f * screenScaleX;
    const float factorY = -4096.0f * screenScaleY;
    result.x = (short)((v[0] - 1.0f) * factorX + 32768.0f);
    result.y = (short)((v[1] - 1.0f) * factorY + 32768.0f);
    result.z = (unsigned int)(v[2] * maxZ);
    return result;
}

#endif
