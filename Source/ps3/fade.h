#ifndef Flurry_ps3_fade_h
#define Flurry_ps3_fade_h
#include <rsx/rsx.h>
#include <cstdint>

class FadeRenderer {
    struct Vertex {
        float x, y;
    };

    Vertex* vertexBuffer;
    uint32_t vertexBufferOffset;
    void* vertexProgramUCode;
    void* fragmentProgramBuffer;
    void* fragmentProgramUCode;
    uint32_t fragmentProgramOffset;

   public:
    FadeRenderer();
    ~FadeRenderer();
    void draw(gcmContextData* context);
};

#endif
