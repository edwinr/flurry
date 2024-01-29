#include "particles.h"
#include <cstdio>
#include <cstring>
#include "../core/Gl_saver.h"
#include "../core/Texture.h"
#include "particles_fpo.h"
#include "particles_vpo.h"

void FlurryRenderer::updateVertices(global_info_t* info) {
    float* positions = (float*)(info->s->seraphimVertices);
    float* texCoords = (float*)(info->s->seraphimTextures);
    float* colors = (float*)(info->s->seraphimColors);
    for (int i = 0; i < NUMSMOKEPARTICLES * 4; ++i) {
        Vertex vertex;
        vertex.x = positions[i * 2];
        vertex.y = positions[i * 2 + 1];
        vertex.u = texCoords[i * 2];
        vertex.v = texCoords[i * 2 + 1];
        vertex.r = colors[i * 4];
        vertex.g = colors[i * 4 + 1];
        vertex.b = colors[i * 4 + 2];
        vertex.a = colors[i * 4 + 3];

        memcpy(&vertexBuffer[i], &vertex, sizeof(Vertex));
    }
}

int FlurryRenderer::init() {
    auto tempTexture = new uint8_t[256 * 256 * 4];
    for (int i = 0; i < 256 * 256; ++i) {
    }
    texture.init(256, 256, tempTexture);
    if (!texture) {
        printf("Failed to create texture\n");
    }

    vertexBuffer =
        (Vertex*)rsxMemalign(128, NUMSMOKEPARTICLES * sizeof(Vertex) * 40);
    rsxAddressToOffset(vertexBuffer, &vertexBufferOffset);

    // vertex shader
    uint32_t vertexProgramSize = 0;
    rsxVertexProgramGetUCode((rsxVertexProgram*)particles_vpo,
                             &vertexProgramUCode, &vertexProgramSize);

    // fragment shader
    uint32_t fragmentProgramSize = 0;
    void* fragmentProgamUCode;
    rsxFragmentProgramGetUCode((rsxFragmentProgram*)particles_fpo,
                               &fragmentProgamUCode, &fragmentProgramSize);

    fragmentProgramBuffer = (u32*)rsxMemalign(64, fragmentProgramSize);
    memcpy(fragmentProgramBuffer, fragmentProgamUCode, fragmentProgramSize);
    rsxAddressToOffset(fragmentProgramBuffer, &fragmentProgramOffset);
    return 0;
}

void FlurryRenderer::draw(gcmContextData* context,
                          global_info_t* flurry,
                          uint32_t displayWidth,
                          uint32_t displayHeight) {
    updateVertices(flurry);

    texture.set(context, 0);

    rsxBindVertexArrayAttrib(context, GCM_VERTEX_ATTRIB_POS, 0,
                             vertexBufferOffset + offsetof(Vertex, x),
                             sizeof(Vertex), 2, GCM_VERTEX_DATA_TYPE_F32,
                             GCM_LOCATION_RSX);
    rsxBindVertexArrayAttrib(context, GCM_VERTEX_ATTRIB_TEX0, 0,
                             vertexBufferOffset + offsetof(Vertex, u),
                             sizeof(Vertex), 2, GCM_VERTEX_DATA_TYPE_F32,
                             GCM_LOCATION_RSX);
    rsxBindVertexArrayAttrib(context, GCM_VERTEX_ATTRIB_COLOR0, 0,
                             vertexBufferOffset + offsetof(Vertex, r),
                             sizeof(Vertex), 4, GCM_VERTEX_DATA_TYPE_F32,
                             GCM_LOCATION_RSX);

    rsxLoadVertexProgram(context, (rsxVertexProgram*)particles_vpo,
                         vertexProgramUCode);
    rsxLoadFragmentProgramLocation(context, (rsxFragmentProgram*)particles_fpo,
                                   fragmentProgramOffset, GCM_LOCATION_RSX);

    rsxSetBlendFunc(context, GCM_SRC_ALPHA, GCM_ONE, GCM_SRC_ALPHA, GCM_ONE);

    if (flurry->s->numQuads > 0) {
        rsxDrawVertexArray(context, GCM_TYPE_QUADS, 0, flurry->s->numQuads * 4);
    }
}
