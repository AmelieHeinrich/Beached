//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-09 03:55:25
//

static const int SHADOW_CASCADE_COUNT = 4;

struct Cascade
{
    int SRVIndex;
    float Split;
    column_major float4x4 ViewProj;
};

struct CascadeBuffer
{
    Cascade Cascades[SHADOW_CASCADE_COUNT];
};
