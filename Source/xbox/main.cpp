#include <hal/video.h>
#include <xgu/xgu.h>
#include <xgu/xgux.h>
#include <memory>
#include "math.h"
#include "input.h"

#include "../core/Gl_saver.h"
#include "../core/Texture.h"


static void initFlurry(global_info_t* flurry_info) {
    flurry_info->flurryRandomSeed = RandFlt(0.0, 300.0);

    flurry_info->numStreams = 5;
    flurry_info->streamExpansion = 100;
    flurry_info->currentColorMode = tiedyeColorMode;

    for (int i = 0; i < MAXNUMPARTICLES; i++) {
        flurry_info->p[i] = new Particle;
    }

    flurry_info->s = new SmokeV;
    InitSmoke(flurry_info->s);

    flurry_info->star = new Star;
    InitStar(flurry_info->star);
    flurry_info->star->rotSpeed = 1.0;

    for (int i = 0; i < 64; i++) {
        flurry_info->spark[i] = new Spark;
        InitSpark(flurry_info->spark[i]);
    }
}

static void destructFlurry(global_info_t* flurry_info) {
    for (int i = 0; i < MAXNUMPARTICLES; i++) {
        delete flurry_info->p[i];
    }

    delete flurry_info->s;
    delete flurry_info->star;
    for (int i = 0; i < 64; i++) {
        delete flurry_info->spark[i];
    }
}

static void* createTexture() {
    const unsigned int width = 256;
    const unsigned int height = 256;
    const unsigned int pitch = width * 2;

    //auto tempTexData = new unsigned char[texture_width * texture_height * 2];
    auto texData = (unsigned char*)MmAllocateContiguousMemoryEx(pitch * height, 0, 0x03FFAFFF, 0, PAGE_WRITECOMBINE | PAGE_READWRITE);
    MakeTexture((unsigned char(*)[256][2])texData);

    //TODO: convert to correct non-linear format

    return texData;
}

static float* alloc_vertex_positions;
static float* alloc_vertex_texcoords;
static float* alloc_vertex_colors;

static void* alloc_texture;

static void initVertexShader(void) {
    XguTransformProgramInstruction vs_program[] = {
        #include "vshader.inl"
    };
    
    uint32_t *p = pb_begin();
    
    p = xgu_set_transform_program_start(p, 0);
    
    p = xgu_set_transform_execution_mode(p, XGU_PROGRAM, XGU_RANGE_MODE_PRIVATE);
    p = xgu_set_transform_program_cxt_write_enable(p, false);
    
    p = xgu_set_transform_program_load(p, 0);
    
    // FIXME: wait for xgu_set_transform_program to get fixed
    for(int i = 0; i < sizeof(vs_program)/16; i++) {
        p = push_command(p, NV097_SET_TRANSFORM_PROGRAM, 4);
        p = push_parameters(p, &vs_program[i].i[0], 4);
    }
    
    pb_end(p);
}

struct ColorVertex {
    float x, y;
    float r, g, b, a;
};

static ColorVertex* overlayVertices;

void initOverlay() {
    overlayVertices = (ColorVertex*)MmAllocateContiguousMemoryEx(sizeof(ColorVertex) * 4, 0, 0x03FFAFFF, 0, PAGE_WRITECOMBINE | PAGE_READWRITE);

    float width = (float)pb_back_buffer_width();
    float height = (float)pb_back_buffer_height();
    float alpha = 0.083333333335f;
    overlayVertices[0] = {-1.0f, 0.0f,          0.0f, 0.0f, 0.0f, alpha};
    overlayVertices[1] = {-1.0f, height + 1.0f, 0.0f, 0.0f, 0.0f, alpha};
    overlayVertices[2] = {width, height + 1.0f, 0.0f, 0.0f, 0.0f, alpha};
    overlayVertices[3] = {width, 0.0f,          0.0f, 0.0f, 0.0f, alpha};
}
void destructOverlay() {
    MmFreeContiguousMemory(overlayVertices);
}
void darkenBackground() {
    auto p = pb_begin();
    #include "color.inl"
    p = xgu_set_blend_func_sfactor(p, XGU_FACTOR_SRC_ALPHA);
    p = xgu_set_blend_func_dfactor(p, XGU_FACTOR_ONE_MINUS_SRC_ALPHA);
    pb_end(p);

    for(int i = 0; i < XGU_ATTRIBUTE_COUNT; i++) {
        xgux_set_attrib_pointer((XguVertexArray)i, (XguVertexArrayType)XGU_FLOAT, 0, 0, NULL);
    }

    xgux_set_attrib_pointer(XGU_VERTEX_ARRAY, (XguVertexArrayType)XGU_FLOAT, 2,  sizeof(ColorVertex), &overlayVertices[0].x);
    xgux_set_attrib_pointer(XGU_COLOR_ARRAY,  (XguVertexArrayType)XGU_FLOAT, 4,  sizeof(ColorVertex), &overlayVertices[0].r);


    xgux_draw_arrays(XGU_QUADS, 0, 4);
}

void drawFlurry(global_info_t* info) {
    auto p = pb_begin();
    #include "color_texture.inl"
    
    p = xgu_set_blend_func_sfactor(p, XGU_FACTOR_SRC_ALPHA);
    p = xgu_set_blend_func_dfactor(p, XGU_FACTOR_ONE);
    
    unsigned int width_shift =  8; // 1 << 8 = 256
    unsigned int height_shift = 8;
    p = xgu_set_texture_offset(p, 0, (void *)((uintptr_t)alloc_texture & 0x03ffffff));
    p = xgu_set_texture_format(p, 0, 2, false, XGU_SOURCE_TEXTURE, 2, XGU_TEXTURE_FORMAT_A8Y8_SWIZZLED, 1, width_shift, height_shift, 0);
    p = xgu_set_texture_address(p, 0, XGU_WRAP, false, XGU_WRAP, false, XGU_CLAMP_TO_EDGE, false, false);
    p = xgu_set_texture_control0(p, 0, true, 0, 0);
    p = xgu_set_texture_filter(p, 0, 0, XGU_TEXTURE_CONVOLUTION_GAUSSIAN, 4, 4, false, false, false, false);
    
    pb_end(p);
    
    for(int i = 0; i < XGU_ATTRIBUTE_COUNT; i++) {
        xgux_set_attrib_pointer((XguVertexArray)i, (XguVertexArrayType)XGU_FLOAT, 0, 0, NULL);
    }
    
    memcpy(alloc_vertex_positions, info->s->seraphimVertices, NUMSMOKEPARTICLES * sizeof(float) * 2 * 4);
    memcpy(alloc_vertex_texcoords, info->s->seraphimTextures, NUMSMOKEPARTICLES * sizeof(float) * 2 * 4);
    memcpy(alloc_vertex_colors,    info->s->seraphimColors,   NUMSMOKEPARTICLES * sizeof(float) * 4 * 4);

    xgux_set_attrib_pointer(XGU_VERTEX_ARRAY,    (XguVertexArrayType)XGU_FLOAT, 2,  sizeof(float) * 2, alloc_vertex_positions);
    xgux_set_attrib_pointer(XGU_TEXCOORD0_ARRAY, (XguVertexArrayType)XGU_FLOAT, 2,  sizeof(float) * 2, alloc_vertex_texcoords);
    xgux_set_attrib_pointer(XGU_COLOR_ARRAY,     (XguVertexArrayType)XGU_FLOAT, 4,  sizeof(float) * 4, alloc_vertex_colors);
    
    if (info->s->numQuads > 0) {
        xgux_draw_arrays(XGU_QUADS, 0, info->s->numQuads * 4);
    }
}

int main(void) {
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
    
    pb_init();
    pb_show_front_screen();
    
    int width = pb_back_buffer_width();
    int height = pb_back_buffer_height();
    
    initVertexShader();
    
    alloc_texture = createTexture();
    alloc_vertex_positions = (float*)MmAllocateContiguousMemoryEx(NUMSMOKEPARTICLES * sizeof(float) * 2 * 4, 0, 0x03FFAFFF, 0, PAGE_WRITECOMBINE | PAGE_READWRITE);
    alloc_vertex_texcoords = (float*)MmAllocateContiguousMemoryEx(NUMSMOKEPARTICLES * sizeof(float) * 2 * 4, 0, 0x03FFAFFF, 0, PAGE_WRITECOMBINE | PAGE_READWRITE);
    alloc_vertex_colors =    (float*)MmAllocateContiguousMemoryEx(NUMSMOKEPARTICLES * sizeof(float) * 4 * 4, 0, 0x03FFAFFF, 0, PAGE_WRITECOMBINE | PAGE_READWRITE);
    initOverlay();
    
    XguMatrix4x4 m_proj;
    mtx_identity(&m_proj);
    float left = 0.0f;
    float right = width;
    float bottom = 0.0f;
    float top = height;
    float near = -1000.0f;
    float far = 1000.0f;
    m_proj.f[0] = 2.0f / (right - left);
    m_proj.f[5] = 2.0f / (top - bottom);
    m_proj.f[10] = -2.0f / (far - near);
    m_proj.f[12] = -((right + left) / (right - left));
    m_proj.f[13] = -((top + bottom) / (top - bottom));
    m_proj.f[14] = -((far + near) / (far - near));
    
    XguMatrix4x4 m_viewport;
    mtx_viewport(&m_viewport, 0, 0, width, height, 0.0f, 0xFFFF);
    mtx_multiply(&m_proj, m_proj, m_viewport);
    
    input_init();
    
    auto info = std::make_unique<global_info_t>();
    initFlurry(info.get());
    GLSetupRC(info.get());
    GLResize(info.get(), width, height);

    for (int i = 0; i < 2; ++i) {
        pb_reset();
        pb_target_back_buffer();
        while (pb_busy());

        auto p = pb_begin();
        p = xgu_set_color_clear_value(p, 0xff000000);
        p = xgu_clear_surface(p, XGU_CLEAR_COLOR);
        pb_end(p);

        while (pb_busy());
        while (pb_finished());
    }

    while(1) {
        input_poll();
        
        if(input_button_down(SDL_CONTROLLER_BUTTON_START))
            break;
        
        pb_wait_for_vbl();
        pb_reset();
        pb_target_back_buffer();
        
        while(pb_busy());
        
        uint32_t *p = pb_begin();
        p = xgu_set_cull_face_enable(p, false);
        p = xgu_set_blend_enable(p, true);
        p = xgu_set_depth_test_enable(p, false);
        p = xgu_set_depth_mask(p, false);
        
        p = xgu_set_transform_constant_load(p, 96);
        p = xgu_set_transform_constant(p, (XguVec4 *)&m_proj, 4);

        pb_end(p);

        darkenBackground();

        GLRenderScene(info.get());
        drawFlurry(info.get());
        
        while(pb_busy());
        while(pb_finished());
    }
    
    input_free();
    
    MmFreeContiguousMemory(alloc_vertex_positions);
    MmFreeContiguousMemory(alloc_vertex_texcoords);
    MmFreeContiguousMemory(alloc_vertex_colors);
    MmFreeContiguousMemory(alloc_texture);
    destructOverlay();
    
    pb_show_debug_screen();
    pb_kill();
    return 0;
}
