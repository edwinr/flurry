#ifndef Flurry_ps3_particles_h
#define Flurry_ps3_particles_h
#include "ps3texture.h"

struct global_info_t;
class FlurryRenderer {
    struct Vertex {
        float x, y;
        float u, v;
        float r, g, b, a;
    };

    PS3Texture texture;
    Vertex* vertexBuffer;
    uint32_t vertexBufferOffset;
    void* vertexProgramUCode;
    void* fragmentProgramBuffer;
    void* fragmentProgramUCode;
    uint32_t fragmentProgramOffset;

    void updateVertices(global_info_t* flurry);

   public:
    int init();
    void draw(gcmContextData* context,
              global_info_t* flurry,
              uint32_t displayWidth,
              uint32_t displayHeight);
};

#endif
