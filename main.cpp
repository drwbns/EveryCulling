#include "OpenGL_helper.h"
#include "EveryCulling.h"

int main() {
    InitializeOpenGL();

    culling::EveryCulling cullingSystem(800, 600);
    InitializeCullingSystem(cullingSystem);

    std::vector<Model> models;
    LoadModels(models);

    GLFWwindow* window = glfwGetCurrentContext();
    RenderLoop(window, cullingSystem, models);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}