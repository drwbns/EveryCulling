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
#include <filesystem>

GLuint CreateShaderProgram();

struct Model;

void RenderModel(const Model& model, const glm::mat4& viewProjection);

void CheckGLError(const char* function);

struct Model {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint shaderProgram;
    GLuint indexCount;
    glm::mat4 modelMatrix;
};

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

culling::Vec3 ExtractTranslationFromMatrix(const glm::mat4& mat) {
    return culling::Vec3{mat[3][0], mat[3][1], mat[3][2]};
}

void LoadModels(std::vector<Model>& models, culling::EveryCulling& cullingSystem) {
    std::cout << "Entering LoadModels function" << std::endl;

    try {
        // Load your models here
        // This is a placeholder implementation for a single triangle model
        Model triangleModel;
        
        glGenVertexArrays(1, &triangleModel.vao);
        glGenBuffers(1, &triangleModel.vbo);
        glGenBuffers(1, &triangleModel.ebo);

        glBindVertexArray(triangleModel.vao);

        // Define vertex data for a simple triangle
        float vertices[] = {
            -0.5f, -0.5f, -3.0f,
             0.5f, -0.5f, -3.0f,
             0.0f,  0.5f, -3.0f
        };

        glBindBuffer(GL_ARRAY_BUFFER, triangleModel.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Define index data
        unsigned int indices[] = {
            0, 1, 2
        };

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleModel.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        triangleModel.indexCount = 3;
        triangleModel.shaderProgram = CreateShaderProgram();
        triangleModel.modelMatrix = glm::mat4(1.0f); // Identity matrix

        models.push_back(triangleModel);

        // Add the triangle to the culling system
        culling::EntityBlockViewer entityViewer = cullingSystem.AllocateNewEntity();
        if (entityViewer.IsValid()) {
            std::cout << "EntityBlockViewer is valid" << std::endl;

            // Calculate AABB for the triangle
            glm::vec3 min = glm::vec3(vertices[0], vertices[1], vertices[2]);
            glm::vec3 max = min;
            for (int i = 1; i < 3; ++i) {
                min = glm::min(min, glm::vec3(vertices[i*3], vertices[i*3+1], vertices[i*3+2]));
                max = glm::max(max, glm::vec3(vertices[i*3], vertices[i*3+1], vertices[i*3+2]));
            }

            // Convert glm::mat4 to culling::Mat4x4
            culling::Mat4x4 cullingModelMatrix = ConvertToCullingMat4x4(triangleModel.modelMatrix);

            culling::Vec3 center((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f, (min.z + max.z) * 0.5f);
            culling::Vec3 aabbMin(min.x, min.y, min.z);
            culling::Vec3 aabbMax(max.x, max.y, max.z);

            // Create culling::Vec3 array for vertices
            std::vector<culling::Vec3> cullingVertices;
            for (int i = 0; i < 3; ++i) {
                cullingVertices.push_back(culling::Vec3(vertices[i*3], vertices[i*3+1], vertices[i*3+2]));
            }

            // Create index array
            std::vector<std::uint32_t> cullingIndices = {0, 1, 2};

            std::cout << "Before UpdateEntityData" << std::endl;
            entityViewer.UpdateEntityData(
                center.data(),
                aabbMin.data(),
                aabbMax.data(),
                cullingModelMatrix.data()
            );
            std::cout << "After UpdateEntityData" << std::endl;

            std::cout << "cullingVertices address: " << (void*)cullingVertices.data() << std::endl;
            std::cout << "cullingVertices capacity: " << cullingVertices.capacity() << std::endl;
            
            if (cullingVertices.empty()) {
                std::cerr << "Error: cullingVertices is empty" << std::endl;
            } else {
                std::cout << "cullingVertices is not empty" << std::endl;
                std::cout << "Vertex count: " << cullingVertices.size() << std::endl;
            }

            // Print debug information
            std::cout << "Entity added to culling system:" << std::endl;
            std::cout << "Vertex count: " << cullingVertices.size() << std::endl;
            std::cout << "Index count: " << cullingIndices.size() << std::endl;
            for (size_t i = 0; i < std::min(cullingVertices.size(), size_t(5)); ++i) {
                std::cout << "Vertex " << i << ": " << cullingVertices[i].x << ", " << cullingVertices[i].y << ", " << cullingVertices[i].z << std::endl;
            }
            for (size_t i = 0; i < std::min(cullingIndices.size(), size_t(10)); ++i) {
                std::cout << "Index " << i << ": " << cullingIndices[i] << std::endl;
            }

            std::cout << "Before calling SetMeshVertexData" << std::endl;
            entityViewer.SetMeshVertexData(
                cullingVertices.data(),
                cullingVertices.size(),
                cullingIndices.data(),
                cullingIndices.size(),
                sizeof(culling::Vec3)
            );
            std::cout << "After calling SetMeshVertexData" << std::endl;
        } else {
            std::cerr << "Failed to allocate entity for culling system" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught in LoadModels: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception caught in LoadModels" << std::endl;
    }

    std::cout << "Exiting LoadModels function" << std::endl;
}

void RenderModels(const culling::EveryCulling& cullingSystem, const std::vector<Model>& models, const glm::mat4& viewProjection) {
    const auto& entityBlocks = cullingSystem.GetActiveEntityBlockList();
    std::cout << "Entity blocks to render: " << entityBlocks.size() << std::endl;

    for (const auto& block : entityBlocks) {
        std::cout << "Entities in block: " << block->mCurrentEntityCount << std::endl;
        for (size_t i = 0; i < block->mCurrentEntityCount; ++i) {
            std::cout << "Entity " << i << " culled: " << block->GetIsCulled(i, 0) << std::endl;
            if (!block->GetIsCulled(i, 0)) {
                // Render the entity
                if (i < models.size()) {
                    RenderModel(models[i], viewProjection);
                    std::cout << "Rendering model " << i << std::endl;
                }
            }
        }
    }
}

void RenderModel(const Model& model, const glm::mat4& viewProjection) {
    glUseProgram(model.shaderProgram);
    
    glm::mat4 mvp = viewProjection * model.modelMatrix;
    GLuint mvpLoc = glGetUniformLocation(model.shaderProgram, "viewProjection");
    if (mvpLoc == GL_INVALID_INDEX) {
        std::cerr << "Failed to get uniform location for viewProjection" << std::endl;
    }
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    std::cout << "Rendering model with " << model.indexCount << " indices" << std::endl;

    glBindVertexArray(model.vao);
    glDrawElements(GL_TRIANGLES, model.indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
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
        std::cerr << "Current working directory: " << std::filesystem::current_path() << std::endl;
        return "";
    }

    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();

    return shaderStream.str();
}

GLuint CreateShaderProgram() {
    std::string exePath = std::filesystem::current_path().string();
    std::string vertexShaderSource = ReadShaderFile(exePath + "/shaders/vertex_shader.glsl");
    std::string fragmentShaderSource = ReadShaderFile(exePath + "/shaders/fragment_shader.glsl");

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

    // Temporarily disable depth testing
    // glEnable(GL_DEPTH_TEST);
}

void RenderLoop(GLFWwindow* window, culling::EveryCulling& cullingSystem, const std::vector<Model>& models) {
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update camera position and view-projection matrix
        glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 viewProjection = projection * view;

        std::cout << "Before culling: Active entity blocks: " << cullingSystem.GetActiveEntityBlockCount() << std::endl;

        // Perform culling
        cullingSystem.PreCullJob();
        cullingSystem.ThreadCullJob(0, cullingSystem.GetTickCount());

        std::cout << "After culling: Active entity blocks: " << cullingSystem.GetActiveEntityBlockCount() << std::endl;

        // Render visible models
        RenderModels(cullingSystem, models, viewProjection);

        // Add debug output
        std::cout << "Rendering triangle at position: " 
                  << models[0].modelMatrix[3][0] << ", " 
                  << models[0].modelMatrix[3][1] << ", " 
                  << models[0].modelMatrix[3][2] << std::endl;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void CheckGLError(const char* function) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error in " << function << ": " << error << std::endl;
    }
}