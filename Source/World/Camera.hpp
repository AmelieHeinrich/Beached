//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-04 00:54:49
//

#pragma once

#include <Core/Common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    Camera() = default;
    ~Camera() = default;

    void Begin();
    void Update(float dt, int width, int height);

    glm::mat4 View() { return mView; }
    glm::mat4 Projection() { return mProjection; }
private:
    glm::mat4 mView = glm::mat4(1.0f);
    glm::mat4 mProjection = glm::mat4(1.0f);
    glm::vec3 mPosition = glm::vec3(0.0f);
    glm::vec3 mForward = glm::vec3(0.0f);
    glm::vec3 mUp = glm::vec3(0.0f);
    glm::vec3 mRight = glm::vec3(0.0f);

    // To calculate forward
    float mPitch = 0.0f;
    float mYaw = 90.0f;

    // Last mouse
    float mLastX = 0.0f;
    float mLastY = 0.0f;
};