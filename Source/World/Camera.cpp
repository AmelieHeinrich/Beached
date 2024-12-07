//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-04 01:42:33
//

#include <World/Camera.hpp>

#include <imgui.h>

void Camera::Begin()
{
    ImVec2 pos = ImGui::GetMousePos();
    mLastX = pos.x;
    mLastY = pos.y;
}

void Camera::Update(float dt, int width, int height)
{
    // Keyboard input
    if (ImGui::IsKeyDown(ImGuiKey_Z))
        mPosition += mForward * dt * 3.0f;
    if (ImGui::IsKeyDown(ImGuiKey_S))
        mPosition -= mForward * dt * 3.0f;
    if (ImGui::IsKeyDown(ImGuiKey_Q))
        mPosition -= mRight * dt * 3.0f;
    if (ImGui::IsKeyDown(ImGuiKey_D))
        mPosition += mRight * dt * 3.0f;

    // Mouse input
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        ImVec2 pos = ImGui::GetMousePos();
        float dx = (pos.x - mLastX) * 0.1f;
        float dy = (pos.y - mLastY) * 0.1f;
    
        mYaw += dx;
        mPitch -= dy;
    }

    // Calculate vectors
    mForward.x = glm::cos(glm::radians(mYaw)) * glm::cos(glm::radians(mPitch));
    mForward.y = glm::sin(glm::radians(mPitch));
    mForward.z = glm::sin(glm::radians(mYaw)) * glm::cos(glm::radians(mPitch));
    mForward = glm::normalize(mForward);

    mRight = glm::normalize(glm::cross(mForward, glm::vec3(0.0f, 1.0f, 0.0f)));
    mUp = glm::normalize(glm::cross(mRight, mForward));

    // Calculate matrices
    mView = glm::lookAt(mPosition, mPosition + mForward, glm::vec3(0.0f, 1.0f, 0.0f));
    mProjection = glm::perspective(glm::radians(90.0f), (float)width / (float)height, CAMERA_NEAR, CAMERA_FAR);
}

Vector<glm::vec4> Camera::Corners() const
{
    glm::mat4 inv = glm::inverse(mProjection * mView);

    Vector<glm::vec4> corners;
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            for (int z = 0; z < 2; z++) {
                // Go from NDC to world
                glm::vec4 point = inv * glm::vec4(
                    2.0f * x - 1.0f,
                    2.0f * y - 1.0f,
                    2.0f * z - 1.0f,
                    1.0f
                );
                point = point / point.w; // Perspective divide
                corners.push_back(point);
            }
        }
    }
    return corners;
}
