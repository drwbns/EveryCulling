#include <build/OpenGL_helper.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "EveryCulling.h"


void InitializeOpenGL() {
    if (!glfwInit()) {
        // Initialization failed
        exit(EXIT_FAILURE);
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Culling Demo", NULL, NULL);
    if (!window) {
        // Window or OpenGL context creation failed
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        // GLEW initialization failed
        exit(EXIT_FAILURE);
    }

    glEnable(GL_DEPTH_TEST);
}

void RenderLoop(GLFWwindow* window, culling::EveryCulling& cullingSystem) {
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update camera and culling system
        cullingSystem.PreCullJob();
        cullingSystem.ThreadCullJob(0, cullingSystem.GetTickCount());
        cullingSystem.WaitToFinishCullJob(0);

        // Render models based on culling results
        // ...

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int main() {
    InitializeOpenGL();

    culling::EveryCulling cullingSystem(800, 600);
    InitializeCullingSystem(cullingSystem);

    GLFWwindow* window = glfwGetCurrentContext();
    RenderLoop(window, cullingSystem);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}