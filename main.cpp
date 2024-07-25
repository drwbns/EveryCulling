#include <OpenGL_helper.h>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "EveryCulling.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

struct Model {
    GLuint vao;           // Vertex Array Object
    GLuint shaderProgram; // Shader program ID
    GLsizei indexCount;   // Number of indices for drawing
    glm::mat4 modelMatrix; // Model's transformation matrix

    Model(GLuint vao, GLuint shaderProgram, GLsizei indexCount, const glm::mat4& modelMatrix)
        : vao(vao), shaderProgram(shaderProgram), indexCount(indexCount), modelMatrix(modelMatrix) {}
};

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
    // Read shader source files
    std::string vertexShaderSource = ReadShaderFile("shaders/vertex.glsl");
    std::string fragmentShaderSource = ReadShaderFile("shaders/fragment.glsl");

    if (vertexShaderSource.empty() || fragmentShaderSource.empty()) {
        return 0;
    }

    // Compile shaders
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource.c_str());
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource.c_str());

    if (!vertexShader || !fragmentShader) {
        return 0;
    }

    // Create and link shader program
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

    // Clean up
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void InitializeOpenGL() {
    if (!glfwInit()) {
        // GLFW Initialization failed
        exit(EXIT_FAILURE);
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Culling Demo", NULL, NULL);
    if (!window) {
        // GLFW Window or OpenGL context creation failed
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        // GLEW Initialization failed
        exit(EXIT_FAILURE);
    }

    glEnable(GL_DEPTH_TEST);
}

void RenderModel(const Model& model, const glm::mat4& viewProjection) {
    // Bind shader program and set uniforms
    glUseProgram(model.shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(model.shaderProgram, "viewProjection"), 1, GL_FALSE, &viewProjection[0][0]);

    // Bind VAO and draw
    glBindVertexArray(model.vao);
    glDrawElements(GL_TRIANGLES, model.indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void RenderLoop(GLFWwindow* window, culling::EveryCulling& cullingSystem, const std::vector<Model>& models) {
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 viewProjection = projection * view;

    const size_t cameraIndex = 0; // Added this line to define cameraIndex

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update camera and culling system
        cullingSystem.PreCullJob();
        cullingSystem.ThreadCullJob(0, cullingSystem.GetTickCount());
        cullingSystem.WaitToFinishCullJob(0);

        // Get active entity blocks
        const auto& activeEntityBlocks = cullingSystem.GetActiveEntityBlockList();

        // Render models based on culling results
        for (const auto& entityBlock : activeEntityBlocks) {
            for (size_t i = 0; i < entityBlock->mCurrentEntityCount; ++i) {
                if (!entityBlock->GetIsCulled(i, cameraIndex)) {
                    RenderModel(models[i], viewProjection);
                }
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void InitializeCullingSystem(culling::EveryCulling& cullingSystem) {
    cullingSystem.SetCameraCount(1);

    // Set up camera parameters
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);
    float fov = 45.0f;
    float aspectRatio = 800.0f / 600.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    // Create view and projection matrices
    glm::mat4 viewMatrix = glm::lookAt(cameraPos, cameraTarget, upVector);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

    // Convert to culling::Mat4x4
    culling::Mat4x4 cullingViewProjectionMatrix;
    std::memcpy(cullingViewProjectionMatrix.data(), glm::value_ptr(viewProjectionMatrix), sizeof(culling::Mat4x4));

    // Set the view projection matrix in the culling system
    cullingSystem.UpdateGlobalDataForCullJob(0, {
        cullingViewProjectionMatrix,
        fov,
        nearPlane,
        farPlane,
        {cameraPos.x, cameraPos.y, cameraPos.z},
        {0.0f, 0.0f, 0.0f, 1.0f} // Assuming no rotation for simplicity
    });
}

void LoadModels(std::vector<Model>& models) {
    // TODO: Implement actual model loading
    // For now, we'll just add a simple triangle as a placeholder
    GLuint vao, vbo, ebo;
    GLuint shaderProgram = CreateShaderProgram(); // You need to implement this function

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 2
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    models.emplace_back(vao, shaderProgram, 3, glm::mat4(1.0f));
}

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