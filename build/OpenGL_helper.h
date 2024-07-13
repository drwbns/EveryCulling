#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
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

void LoadModels() {
    // Load your models here
}

void RenderModels(const culling::EveryCulling& cullingSystem) {
    const auto& entityBlocks = cullingSystem.GetActiveEntityBlockList();
    for (const auto& block : entityBlocks) {
        for (size_t i = 0; i < block->mCurrentEntityCount; ++i) {
            if (!block->GetIsCulled(i, 0)) {
                // Render the entity
            }
        }
    }
}

void InitializeCullingSystem(culling::EveryCulling& cullingSystem) {
    cullingSystem.SetCameraCount(1);
    cullingSystem.SetViewProjectionMatrix(0, /* your view projection matrix */);
    cullingSystem.SetFieldOfViewInDegree(0, 45.0f);
    cullingSystem.SetCameraNearFarClipPlaneDistance(0, 0.1f, 100.0f);
    cullingSystem.SetCameraWorldPosition(0, culling::Vec3{ 0.0f, 0.0f, 5.0f });
    cullingSystem.SetCameraRotation(0, culling::Vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
}