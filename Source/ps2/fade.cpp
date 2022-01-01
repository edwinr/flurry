#include "fade.h"
#include "convert.h"
#include <draw.h>
#include <draw3d.h>

qword_t* darkenBackground(qword_t* q) {
    const float alpha = 0.083333333335f;
    
    VECTOR colors[] = {{0.0f, 0.0f, 0.0f, alpha},
                       {0.0f, 0.0f, 0.0f, alpha},
                       {0.0f, 0.0f, 0.0f, alpha},
                       {0.0f, 0.0f, 0.0f, alpha}};
    VECTOR positions[] = {{0, 0, 0, 1},
                          {2, 0, 0, 1},
                          {2, 2, 0, 1},
                          {0, 2, 0, 1}};

    xyz_t convertedPositions[4];
    color_t convertedColors[4];

    for (int i=0; i<4; ++i) {
        convertedPositions[i] = convertXyz(positions[i]);
    }
    draw_convert_rgbaq(convertedColors, 4, (vertex_f_t*)positions, (color_f_t*)colors);

    blend_t blend;
    blend.color1 = BLEND_COLOR_SOURCE;
    blend.color2 = BLEND_COLOR_DEST;
    blend.alpha  = BLEND_ALPHA_SOURCE;
    blend.color3 = BLEND_COLOR_DEST;
    blend.fixed_alpha = 0x80;
    q = draw_alpha_blending(q, 0, &blend);

    prim_t prim;
    prim.type = PRIM_TRIANGLE_FAN;
    prim.shading = PRIM_SHADE_GOURAUD;
    prim.mapping = DRAW_DISABLE;
    prim.fogging = DRAW_DISABLE;
    prim.blending = DRAW_ENABLE;
    prim.antialiasing = DRAW_DISABLE;
    prim.mapping_type = PRIM_MAP_ST;
    prim.colorfix = PRIM_UNFIXED;

    color_t color = {0};
    q = draw_prim_start(q, 0, &prim, &color);
    for (int i=0; i<4; ++i) {
        q->dw[0] = convertedColors[i].rgbaq;
        q->dw[1] = convertedPositions[i].xyz;
        q++;
    }
    q = draw_prim_end(q, 2, DRAW_RGBAQ_REGLIST);

    return q;
}