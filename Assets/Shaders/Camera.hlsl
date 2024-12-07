//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-07 17:53:28
//

struct Camera
{
    column_major float4x4 View;
    column_major float4x4 Projection;
    float3 Position;
    float Pad;
};

static const float CAMERA_NEAR = 0.1f;
static const float CAMERA_FAR = 500.0f;
static const float SHADOW_CASCADE_LEVELS[4] = { CAMERA_FAR / 50.0f, CAMERA_FAR / 25.0f, CAMERA_FAR / 10.0f, CAMERA_FAR / 2.0f };
