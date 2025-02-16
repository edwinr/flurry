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

    int width = 256, height = 256;
#ifdef FLURRY_TINY_TEXTURES
    width = 32;
    height = 32;
    int writeIndex = 0;

    
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            int index = (y * 256 + x) * 2;
            bigTextureArray[writeIndex] = bigTextureArray[index] / 2;
            bigTextureArray[writeIndex + 1] = bigTextureArray[index + 1];
            writeIndex += 2;
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width, height, 0,
                 GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, bigTextureArray);

    int level = 1;
    while (width > 1 && height > 1) {
        int levelOffset = writeIndex;

        for (int y = 0; y < width; y += 2) {
            for (int x = 0; x < width; x += 2) {
                int offset = (y * height + x) * 2;
                int luminance = bigTextureArray[offset] +
                                bigTextureArray[offset + 2] +
                                bigTextureArray[offset + width * 2] +
                                bigTextureArray[offset + width * 2 + 2];
                int alpha = bigTextureArray[offset + 1] +
                            bigTextureArray[offset + 3] +
                            bigTextureArray[offset + width * 2 + 1] +
                            bigTextureArray[offset + width * 2 + 3];

                bigTextureArray[writeIndex++] = luminance / 4;
                bigTextureArray[writeIndex++] = alpha / 4;
            }
        }

        width /= 2;
        height /= 2;

        glTexImage2D(GL_TEXTURE_2D, level, GL_LUMINANCE_ALPHA, width, height, 0,
                     GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
                     &bigTextureArray[levelOffset]);
        level++;
    }

#else
    gluBuild2DMipmaps(GL_TEXTURE_2D, 2, width, height, GL_LUMINANCE_ALPHA,
                      GL_UNSIGNED_BYTE, bigTextureArray);
#endif
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
    glOrtho(0, width, 0, height, -100.0, 100.0);
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
    glOrtho(0, width, 0, height, -100.0, 100.0);
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

