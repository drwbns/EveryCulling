#include "OpenGL_helper.h"
#include "EveryCulling.h"
#include <GLFW/glfw3.h>
#include <vector>

#define GLEW_STATIC

int main() {
    InitializeOpenGL();

    culling::EveryCulling cullingSystem(800, 600);
    cullingSystem.SetDebugOutput(true);  // Enable debug output
    InitializeCullingSystem(cullingSystem);
    std::cout << "Culling system initialized" << std::endl;

    std::vector<Model> models;
    LoadModels(models, cullingSystem);  // Pass cullingSystem here

    GLFWwindow* window = glfwGetCurrentContext();
    RenderLoop(window, cullingSystem, models);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}