#include "../core/Gl_saver.h"
#include "../core/Texture.h"

#include <GLFW/glfw3.h>
#include <memory>

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

int main() {
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(800, 600, "Flurry", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    auto info = std::make_unique<global_info_t>();
    ::info = info.get();
    initFlurry(info.get());

    MakeTexture();

    int width = 800, height = 600;
    GLResize(width, height);
    GLSetupRC();

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    double oldFrameTime = TimeInSecondsSinceStart();

    while (!glfwWindowShouldClose(window)) {
        double newFrameTime = TimeInSecondsSinceStart();
        double alpha = 5.0 * (newFrameTime - oldFrameTime);
        if (alpha > 0.2)
            alpha = 0.2;
        oldFrameTime = newFrameTime;

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0, 0.0, 0.0, alpha);
        glRectd(0, 0, width, height);

        glfwGetWindowSize(window, &width, &height);
        glViewport(0.0, 0.0, width, height);
        GLResize(width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, 0, height, 0.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        GLRenderScene();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    destructFlurry(info.get());

    glfwTerminate();
}
