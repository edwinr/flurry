#include "../core/Gl_saver.h"
#include "../core/Texture.h"

#include <GLFW/glfw3.h>
#include <memory>

#include "../opengl/FlurryOpenGL.h"

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
    initDefaultFlurry(info.get(), width, height);

    FlurryOpenGL flurryOpenGL;

    flurryOpenGL.init(width, height);
    GLSetupRC(info.get());

    double oldFrameTime = TimeInSecondsSinceStart();

    while (!glfwWindowShouldClose(window)) {
        double newFrameTime = TimeInSecondsSinceStart();
        double alpha = 5.0 * (newFrameTime - oldFrameTime);
        if (alpha > 0.2)
            alpha = 0.2;

        glfwGetWindowSize(window, &width, &height);
        flurryOpenGL.resize(width, height);
        GLResize(info.get(), width, height);

        flurryOpenGL.darkenBackground(alpha);
        
        GLRenderScene(info.get(), newFrameTime, newFrameTime - oldFrameTime);
        flurryOpenGL.drawFlurryParticles(
            info->s->numQuads * 4, info->s->seraphimVertices,
            info->s->seraphimColors, info->s->seraphimTextures);

        
        glfwSwapBuffers(window);
        glfwPollEvents();

        oldFrameTime = newFrameTime;
    }

    destructFlurry(info.get());

    glfwTerminate();
}
