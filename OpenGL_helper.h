#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "EveryCulling.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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