//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-04 00:54:49
//

#pragma once

#include <Core/Common.hpp>
#include <Physics/Volume.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

constexpr float CAMERA_FAR = 150.0f;
constexpr float CAMERA_NEAR = 0.1f;

struct Plane
{
    glm::vec3 Normal;
    float Distance;
};

class Camera
{
public:
    Camera() = default;
    ~Camera() = default;

    void Begin();
    void Update(float dt, int width, int height);

    glm::mat4 View() const { return mView; }
    glm::mat4 Projection() const { return mProjection; }
    glm::vec3 Position() const { return mPosition; }
    
    Vector<glm::vec4> Corners() const;
    Vector<glm::vec4> CornersForCascade(float near, float far) const;
    Array<Plane, 6> Planes() const;

    static Array<Plane, 6> FrustumPlanes(glm::mat4 projView);
    static Vector<glm::vec4> FrustumCorners(glm::mat4 view, glm::mat4 proj);

    static bool IsBoxOutsidePlane(const Plane& plane, const Box& box);
    static bool IsBoxOutsidePlane(const Plane& plane, const Box& box, const glm::mat4& transform);
    bool IsBoxInFrustum(const Box& box) const;
    bool IsBoxInFrustum(const Box& box, const glm::mat4& transform) const;
    static bool IsBoxInFrustum(const glm::mat4& projView, const Box& box, const glm::mat4& transform);

    void FreezeFrustum(bool freeze);
private:
    glm::mat4 mView = glm::mat4(1.0f);
    glm::mat4 mProjection = glm::mat4(1.0f);
    glm::vec3 mPosition = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 mForward = glm::vec3(0.0f);
    glm::vec3 mUp = glm::vec3(0.0f);
    glm::vec3 mRight = glm::vec3(0.0f);

    // To calculate forward
    float mPitch = 0.0f;
    float mYaw = -90.0f;

    // Last mouse
    float mLastX = 0.0f;
    float mLastY = 0.0f;

    // Saved frustum
    bool mFreezeFrustum = false;
    Array<Plane, 6> mSavedFrustum;
    int mWidth;
    int mHeight;
};
