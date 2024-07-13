#include <build/OpenGL_helper.h>

int main() {
    InitializeOpenGL();

    culling::EveryCulling cullingSystem(800, 600);
    InitializeCullingSystem(cullingSystem);

    GLFWwindow* window = glfwGetCurrentContext();
    LoadModels();
    RenderLoop(window, cullingSystem);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}