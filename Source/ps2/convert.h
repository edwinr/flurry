#ifndef Flurry_ps2_convert_h
#define Flurry_ps2_convert_h

#include <math3d.h>
#include <draw_types.h>

const int screenWidth = 640;
const int screenHeight = 512;
const float screenScaleX = 1.25f;
const float screenScaleY = 1.0f;

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
