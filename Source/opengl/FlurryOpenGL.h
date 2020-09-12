#ifndef Flurry_opengl_FlurryOpenGL_h
#define Flurry_opengl_FlurryOpenGL_h

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#ifdef _WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>
#endif

class FlurryOpenGL {
    GLuint textureId;
    int width, height;
    GLuint createGLTexture();
   public:
    ~FlurryOpenGL();

    void init(int width, int height);
    void resize(int width, int height);
    void darkenBackground(float alpha);
    void drawFlurryParticles(int numPrimitives,
                             void* positionPtr,
                             void* colorPtr,
                             void* texcoordPtr);
};

#endif
