#include "fade.h"
#include <cstring>
#include "fade_fpo.h"
#include "fade_vpo.h"
#include <cstdio>

FadeRenderer::FadeRenderer() {
    const FadeRenderer::Vertex vertices[] = {
        {-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f}};
    vertexBuffer = (Vertex*)rsxMemalign(128, sizeof(vertices));
    std::memcpy(vertexBuffer, vertices, sizeof(vertices));
    rsxAddressToOffset(vertexBuffer, &vertexBufferOffset);

    // vertex shader
    uint32_t vertexProgramSize = 0;
    rsxVertexProgramGetUCode((rsxVertexProgram*)fade_vpo, &vertexProgramUCode,
                             &vertexProgramSize);

    // fragment shader
    uint32_t fragmentProgramSize = 0;
    void* fragmentProgamUCode;
    rsxFragmentProgramGetUCode((rsxFragmentProgram*)fade_fpo,
                               &fragmentProgamUCode, &fragmentProgramSize);

    fragmentProgramBuffer = (u32*)rsxMemalign(64, fragmentProgramSize);
    std::memcpy(fragmentProgramBuffer, fragmentProgamUCode,
                fragmentProgramSize);
    rsxAddressToOffset(fragmentProgramBuffer, &fragmentProgramOffset);
}

FadeRenderer::~FadeRenderer() {
    rsxFree(vertexBuffer);
    rsxFree(fragmentProgramBuffer);
}

void FadeRenderer::draw(gcmContextData* context) {
    rsxBindVertexArrayAttrib(context, GCM_VERTEX_ATTRIB_POS, 0,
                             vertexBufferOffset + offsetof(Vertex, x),
                             sizeof(Vertex), 2, GCM_VERTEX_DATA_TYPE_F32,
                             GCM_LOCATION_RSX);
    rsxBindVertexArrayAttrib(context, GCM_VERTEX_ATTRIB_TEX0, 0, 0,
                             sizeof(Vertex), 0, GCM_VERTEX_DATA_TYPE_F32,
                             GCM_LOCATION_RSX);
    rsxBindVertexArrayAttrib(context, GCM_VERTEX_ATTRIB_COLOR0, 0, 0,
                             sizeof(Vertex), 0, GCM_VERTEX_DATA_TYPE_F32,
                             GCM_LOCATION_RSX);

    rsxLoadVertexProgram(context, (rsxVertexProgram*)fade_vpo,
                         vertexProgramUCode);
    rsxLoadFragmentProgramLocation(context, (rsxFragmentProgram*)fade_fpo,
                                   fragmentProgramOffset, GCM_LOCATION_RSX);

    rsxSetBlendFunc(context, GCM_SRC_ALPHA, GCM_ONE_MINUS_SRC_ALPHA,
                    GCM_SRC_ALPHA, GCM_ONE_MINUS_SRC_ALPHA);
    rsxDrawVertexArray(context, GCM_TYPE_QUADS, 0, 4);
}
