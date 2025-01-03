//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-07 17:53:28
//

struct Camera
{
    column_major float4x4 View;
    column_major float4x4 Projection;
    column_major float4x4 InvView;
    float3 Position;
    float Pad;
};

static const float CAMERA_NEAR = 0.1f;
static const float CAMERA_FAR = 150.0f;
