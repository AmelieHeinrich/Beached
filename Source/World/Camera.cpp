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
    // Update frustum
    mSavedFrustum = Planes();

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

Array<Plane, 6> Camera::FrustumPlanes(glm::mat4 projView)
{
    glm::mat4 projViewMatrix = projView;

    Array<Plane, 6> planes;

    // Left Plane
    planes[0].Normal.x = projViewMatrix[0][3] + projViewMatrix[0][0];
    planes[0].Normal.y = projViewMatrix[1][3] + projViewMatrix[1][0];
    planes[0].Normal.z = projViewMatrix[2][3] + projViewMatrix[2][0];
    planes[0].Distance = projViewMatrix[3][3] + projViewMatrix[3][0];

    // Right Plane
    planes[1].Normal.x = projViewMatrix[0][3] - projViewMatrix[0][0];
    planes[1].Normal.y = projViewMatrix[1][3] - projViewMatrix[1][0];
    planes[1].Normal.z = projViewMatrix[2][3] - projViewMatrix[2][0];
    planes[1].Distance = projViewMatrix[3][3] - projViewMatrix[3][0];

    // Bottom Plane
    planes[2].Normal.x = projViewMatrix[0][3] + projViewMatrix[0][1];
    planes[2].Normal.y = projViewMatrix[1][3] + projViewMatrix[1][1];
    planes[2].Normal.z = projViewMatrix[2][3] + projViewMatrix[2][1];
    planes[2].Distance = projViewMatrix[3][3] + projViewMatrix[3][1];

    // Top Plane
    planes[3].Normal.x = projViewMatrix[0][3] - projViewMatrix[0][1];
    planes[3].Normal.y = projViewMatrix[1][3] - projViewMatrix[1][1];
    planes[3].Normal.z = projViewMatrix[2][3] - projViewMatrix[2][1];
    planes[3].Distance = projViewMatrix[3][3] - projViewMatrix[3][1];

    // Near Plane
    planes[4].Normal.x = projViewMatrix[0][3] + projViewMatrix[0][2];
    planes[4].Normal.y = projViewMatrix[1][3] + projViewMatrix[1][2];
    planes[4].Normal.z = projViewMatrix[2][3] + projViewMatrix[2][2];
    planes[4].Distance = projViewMatrix[3][3] + projViewMatrix[3][2];

    // Far Plane
    planes[5].Normal.x = projViewMatrix[0][3] - projViewMatrix[0][2];
    planes[5].Normal.y = projViewMatrix[1][3] - projViewMatrix[1][2];
    planes[5].Normal.z = projViewMatrix[2][3] - projViewMatrix[2][2];
    planes[5].Distance = projViewMatrix[3][3] - projViewMatrix[3][2];

    // Normalize all planes
    for (auto& plane : planes) {
        float length = glm::length(plane.Normal);
        plane.Normal /= length;
        plane.Distance /= length;
    }
    return planes;
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

Array<Plane, 6> Camera::Planes() const
{
    if (mFreezeFrustum)
        return mSavedFrustum;

    glm::mat4 projViewMatrix = mProjection * mView;

    Array<Plane, 6> planes;

    // Left Plane
    planes[0].Normal.x = projViewMatrix[0][3] + projViewMatrix[0][0];
    planes[0].Normal.y = projViewMatrix[1][3] + projViewMatrix[1][0];
    planes[0].Normal.z = projViewMatrix[2][3] + projViewMatrix[2][0];
    planes[0].Distance = projViewMatrix[3][3] + projViewMatrix[3][0];

    // Right Plane
    planes[1].Normal.x = projViewMatrix[0][3] - projViewMatrix[0][0];
    planes[1].Normal.y = projViewMatrix[1][3] - projViewMatrix[1][0];
    planes[1].Normal.z = projViewMatrix[2][3] - projViewMatrix[2][0];
    planes[1].Distance = projViewMatrix[3][3] - projViewMatrix[3][0];

    // Bottom Plane
    planes[2].Normal.x = projViewMatrix[0][3] + projViewMatrix[0][1];
    planes[2].Normal.y = projViewMatrix[1][3] + projViewMatrix[1][1];
    planes[2].Normal.z = projViewMatrix[2][3] + projViewMatrix[2][1];
    planes[2].Distance = projViewMatrix[3][3] + projViewMatrix[3][1];

    // Top Plane
    planes[3].Normal.x = projViewMatrix[0][3] - projViewMatrix[0][1];
    planes[3].Normal.y = projViewMatrix[1][3] - projViewMatrix[1][1];
    planes[3].Normal.z = projViewMatrix[2][3] - projViewMatrix[2][1];
    planes[3].Distance = projViewMatrix[3][3] - projViewMatrix[3][1];

    // Near Plane
    planes[4].Normal.x = projViewMatrix[0][3] + projViewMatrix[0][2];
    planes[4].Normal.y = projViewMatrix[1][3] + projViewMatrix[1][2];
    planes[4].Normal.z = projViewMatrix[2][3] + projViewMatrix[2][2];
    planes[4].Distance = projViewMatrix[3][3] + projViewMatrix[3][2];

    // Far Plane
    planes[5].Normal.x = projViewMatrix[0][3] - projViewMatrix[0][2];
    planes[5].Normal.y = projViewMatrix[1][3] - projViewMatrix[1][2];
    planes[5].Normal.z = projViewMatrix[2][3] - projViewMatrix[2][2];
    planes[5].Distance = projViewMatrix[3][3] - projViewMatrix[3][2];

    // Normalize all planes
    for (auto& plane : planes) {
        float length = glm::length(plane.Normal);
        plane.Normal /= length;
        plane.Distance /= length;
    }
    return planes;
}

bool Camera::IsBoxOutsidePlane(const Plane& plane, const Box& box)
{
    glm::vec3 positiveVertex = box.Min;
    glm::vec3 negativeVertex = box.Max;

    if (plane.Normal.x >= 0) {
        positiveVertex.x = box.Max.x;
        negativeVertex.x = box.Min.x;
    }
    if (plane.Normal.y >= 0) {
        positiveVertex.y = box.Max.y;
        negativeVertex.y = box.Min.y;
    }
    if (plane.Normal.z >= 0) {
        positiveVertex.z = box.Max.z;
        negativeVertex.z = box.Min.z;
    }

    // Test the negative vertex against the plane
    if (glm::dot(plane.Normal, negativeVertex) + plane.Distance > 0) {
        return true;
    }
    return false;
}

bool Camera::IsBoxOutsidePlane(const Plane& plane, const Box& box, const glm::mat4& transform)
{
    glm::vec3 corners[8] = {
        glm::vec3(box.Min.x, box.Min.y, box.Min.z),
        glm::vec3(box.Min.x, box.Min.y, box.Max.z),
        glm::vec3(box.Min.x, box.Max.y, box.Min.z),
        glm::vec3(box.Min.x, box.Max.y, box.Max.z),
        glm::vec3(box.Max.x, box.Min.y, box.Min.z),
        glm::vec3(box.Max.x, box.Min.y, box.Max.z),
        glm::vec3(box.Max.x, box.Max.y, box.Min.z),
        glm::vec3(box.Max.x, box.Max.y, box.Max.z),
    };

    // Transform each corner by the matrix
    for (auto& corner : corners) {
        corner = glm::vec3(transform * glm::vec4(corner, 1.0f));
    }

    // Check if all corners are outside the plane
    bool allOutside = true;
    for (const auto& corner : corners) {
        if (glm::dot(plane.Normal, corner) + plane.Distance > 0) {
            allOutside = false;
            break; // At least one point is inside or intersecting the plane
        }
    }

    return allOutside; // If all corners are outside, the OBB is outside the plane
}

bool Camera::IsBoxInFrustum(const Box& box) const
{
    for (const auto& plane : Planes()) {
        if (IsBoxOutsidePlane(plane, box)) {
            return false;
        }
    }
    return true;
}

bool Camera::IsBoxInFrustum(const Box& box, const glm::mat4& transform) const
{
    for (const auto& plane : Planes()) {
        if (IsBoxOutsidePlane(plane, box, transform)) {
            return false;
        }
    }
    return true;
}

bool Camera::IsBoxInFrustum(const glm::mat4& projView, const Box& box, const glm::mat4& transform)
{
    for (const auto& plane : FrustumPlanes(projView)) {
        if (IsBoxOutsidePlane(plane, box, transform)) {
            return false;
        }
    }
    return true;
}

void Camera::FreezeFrustum(bool freeze)
{
    mFreezeFrustum = freeze;
}
