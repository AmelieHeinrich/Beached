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
    return FrustumCorners(mView, mProjection);
}

Vector<glm::vec4> Camera::FrustumCorners(glm::mat4 view, glm::mat4 proj)
{
    glm::mat4 inv = glm::inverse(view * proj);

    Vector<glm::vec4> corners = {
        glm::vec4(-1.0f,  1.0f, 0.0f, 1.0f),
        glm::vec4( 1.0f,  1.0f, 0.0f, 1.0f),
        glm::vec4( 1.0f, -1.0f, 0.0f, 1.0f),
        glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f),
        glm::vec4(-1.0f,  1.0f, 1.0f, 1.0f),
        glm::vec4( 1.0f,  1.0f, 1.0f, 1.0f),
        glm::vec4( 1.0f, -1.0f, 1.0f, 1.0f),
        glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f),
    };

    // To convert from world space to NDC space, multiply by the inverse of the camera matrix (projection * view) then perspective divide
    // Not sure I 100% understand the math here, TODO: study
    for (int i = 0; i < 8; i++) {
        glm::vec4 h = inv * corners[i];
        h.x /= h.w;
        h.y /= h.w;
        h.z /= h.w;
        corners[i] = h;
    }
    return corners;
}
