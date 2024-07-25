#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "EveryCulling.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// Function to create a view matrix
glm::mat4 CreateViewMatrix(const glm::vec3& cameraPos, const glm::vec3& cameraTarget, const glm::vec3& upVector) {
    return glm::lookAt(cameraPos, cameraTarget, upVector);
}

// Function to create a projection matrix
glm::mat4 CreateProjectionMatrix(float fov, float aspectRatio, float nearPlane, float farPlane) {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

culling::Mat4x4 ConvertToCullingMat4x4(const glm::mat4& mat) {
    culling::Mat4x4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result[i][j] = mat[i][j];
        }
    }
    return result;
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

    // Set up camera parameters
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);
    float fov = 45.0f;
    float aspectRatio = 800.0f / 600.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    // Create view and projection matrices
    glm::mat4 viewMatrix = CreateViewMatrix(cameraPos, cameraTarget, upVector);
    glm::mat4 projectionMatrix = CreateProjectionMatrix(fov, aspectRatio, nearPlane, farPlane);
    glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

    // Convert to culling::Mat4x4
    culling::Mat4x4 cullingViewProjectionMatrix = ConvertToCullingMat4x4(viewProjectionMatrix);

    // Set the view projection matrix in the culling system
    cullingSystem.SetViewProjectionMatrix(0, cullingViewProjectionMatrix);
    cullingSystem.SetFieldOfViewInDegree(0, fov);
    cullingSystem.SetCameraNearFarClipPlaneDistance(0, nearPlane, farPlane);
    cullingSystem.SetCameraWorldPosition(0, culling::Vec3{ cameraPos.x, cameraPos.y, cameraPos.z });
    cullingSystem.SetCameraRotation(0, culling::Vec4{ 0.0f, 0.0f, 0.0f, 1.0f }); // Assuming no rotation for simplicity
}

GLuint CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader compilation error: " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

std::string ReadShaderFile(const std::string& filePath) {
    std::ifstream shaderFile(filePath);
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }

    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();

    return shaderStream.str();
}

GLuint CreateShaderProgram() {
    std::string vertexShaderSource = ReadShaderFile("vertex_shader.glsl");
    std::string fragmentShaderSource = ReadShaderFile("fragment_shader.glsl");

    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource.c_str());
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource.c_str());

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader program linking error: " << infoLog << std::endl;
        glDeleteProgram(shaderProgram);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void InitializeOpenGL() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "EveryCulling Demo", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }

    glEnable(GL_DEPTH_TEST);
}

void RenderModel(const Model& model, const glm::mat4& viewProjection) {
    glUseProgram(model.shaderProgram);

    GLint mvpLoc = glGetUniformLocation(model.shaderProgram, "MVP");
    glm::mat4 mvp = viewProjection * model.modelMatrix;
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    glBindVertexArray(model.vao);
    glDrawElements(GL_TRIANGLES, model.indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void RenderLoop(GLFWwindow* window, culling::EveryCulling& cullingSystem, const std::vector<Model>& models) {
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update camera position and view-projection matrix
        glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 viewProjection = projection * view;

        // Update culling system
        culling::EveryCulling::GlobalDataForCullJob cullingSettingParameters;
        cullingSettingParameters.mViewProjectionMatrix = ConvertToCullingMat4x4(viewProjection);
        cullingSettingParameters.mCameraWorldPosition = culling::Vec3{ cameraPos.x, cameraPos.y, cameraPos.z };
        cullingSettingParameters.mFieldOfViewInDegree = 45.0f;
        cullingSettingParameters.mCameraNearPlaneDistance = 0.1f;
        cullingSettingParameters.mCameraFarPlaneDistance = 100.0f;
        cullingSettingParameters.mCameraRotation = culling::Vec4{ 0.0f, 0.0f, 0.0f, 1.0f };

        cullingSystem.UpdateGlobalDataForCullJob(0, cullingSettingParameters);

        // Perform culling
        cullingSystem.PreCullJob();
        cullingSystem.ThreadCullJob(0, cullingSystem.GetTickCount());

        // Render visible models
        const auto& entityBlocks = cullingSystem.GetActiveEntityBlockList();
        for (const auto& block : entityBlocks) {
            for (size_t i = 0; i < block->mCurrentEntityCount; ++i) {
                if (!block->GetIsCulled(i, 0)) {
                    // Assuming the model index corresponds to the entity index
                    if (i < models.size()) {
                        RenderModel(models[i], viewProjection);
                    }
                }
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void LoadModels(std::vector<Model>& models) {
    // Load your models here
    // This is a placeholder implementation
    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    // Define vertex data for a simple triangle
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Define index data
    unsigned int indices[] = {
        0, 1, 2
    };

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    GLuint shaderProgram = CreateShaderProgram();

    models.emplace_back(vao, shaderProgram, 3, glm::mat4(1.0f));
}

struct Model {
    GLuint vao;           // Vertex Array Object
    GLuint shaderProgram; // Shader program ID
    GLsizei indexCount;   // Number of indices for drawing
    glm::mat4 modelMatrix; // Model's transformation matrix

    Model(GLuint vao, GLuint shaderProgram, GLsizei indexCount, const glm::mat4& modelMatrix)
        : vao(vao), shaderProgram(shaderProgram), indexCount(indexCount), modelMatrix(modelMatrix) {}
};