#include "FlurryOpenGL.h"
#include "../core/Texture.h"

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif


GLuint FlurryOpenGL::createGLTexture() {
    auto bigTextureArray = new unsigned char[256 * 256 * 2];
    MakeTexture((unsigned char(*)[256][2])bigTextureArray);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    // Set the tiling mode (this is generally always GL_REPEAT).
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Set the filtering.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_NEAREST);

    gluBuild2DMipmaps(GL_TEXTURE_2D, 2, 256, 256, GL_LUMINANCE_ALPHA,
                      GL_UNSIGNED_BYTE, bigTextureArray);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    delete[] bigTextureArray;

    return texId;
}

FlurryOpenGL::~FlurryOpenGL() {
    glDeleteTextures(1, &textureId);
}

void FlurryOpenGL::init(int width, int height) {
    resize(width, height);
    textureId = createGLTexture();

    glDisable(GL_DEPTH_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);
    glEnable(GL_ALPHA_TEST);
    glShadeModel(GL_FLAT);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, 0.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void FlurryOpenGL::resize(int width, int height) {
    this->width = width;
    this->height = height;

    glViewport(0.0, 0.0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, 0.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void FlurryOpenGL::darkenBackground(float alpha) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0, 0.0, 0.0, alpha);
    glRectd(0, 0, width, height);
}

void FlurryOpenGL::drawFlurryParticles(int numPrimitives,
                                       void* positionPtr,
                                       void* colorPtr,
                                       void* texcoordPtr) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_TEXTURE_2D);
    glColorPointer(4, GL_FLOAT, 0, colorPtr);
    glVertexPointer(2, GL_FLOAT, 0, positionPtr);
    glTexCoordPointer(2, GL_FLOAT, 0, texcoordPtr);
    glDrawArrays(GL_QUADS, 0, numPrimitives);
    glDisable(GL_TEXTURE_2D);
}

