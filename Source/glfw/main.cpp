#include "../core/Gl_saver.h"
#include "../core/Texture.h"

#include <GLFW/glfw3.h>
#include <memory>

#include "../opengl/FlurryOpenGL.h"

static void initFlurry(global_info_t* flurry_info, int width, int height) {
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

    for (int i = 0; i < numParticles; i++) {
        InitParticle(flurry_info->p[i], width, height);
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
    glfwSwapInterval(1);

    int width = 800, height = 600;
    auto info = std::make_unique<global_info_t>();
    initFlurry(info.get(), width, height);

    FlurryOpenGL flurryOpenGL;

    flurryOpenGL.init(width, height);
    GLSetupRC(info.get());

    double oldFrameTime = TimeInSecondsSinceStart();

    while (!glfwWindowShouldClose(window)) {
        double newFrameTime = TimeInSecondsSinceStart();
        double alpha = 5.0 * (newFrameTime - oldFrameTime);
        if (alpha > 0.2)
            alpha = 0.2;
        oldFrameTime = newFrameTime;

        glfwGetWindowSize(window, &width, &height);
        flurryOpenGL.resize(width, height);
        GLResize(info.get(), width, height);

        flurryOpenGL.darkenBackground(alpha);
        
        GLRenderScene(info.get());
        flurryOpenGL.drawFlurryParticles(
            info->s->numQuads * 4, info->s->seraphimVertices,
            info->s->seraphimColors, info->s->seraphimTextures);

        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    destructFlurry(info.get());

    glfwTerminate();
}
