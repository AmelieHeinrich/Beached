//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-04 03:38:12
//

#pragma once

#include <Core/Common.hpp>
#include <RHI/RHI.hpp>

#include <cgltf/cgltf.h>
#include <glm/glm.hpp>

class Asset;

struct Vertex
{
    glm::vec3 Position;
    glm::vec2 UV;
};

struct GLTFMaterial
{
    Ref<Asset> Albedo;
    View::Ref AlbedoView;
};

struct GLTFPrimitive
{
    Buffer::Ref VertexBuffer;
    Buffer::Ref IndexBuffer;

    UInt32 VertexCount;
    UInt32 IndexCount;
    int MaterialIndex;
};

struct GLTFNode
{
    Vector<GLTFPrimitive> Primitives;
    Array<Buffer::Ref, FRAMES_IN_FLIGHT> ModelBuffer;

    String Name;
    glm::mat4 Transform;
    GLTFNode* Parent;
    Vector<GLTFNode*> Children;

    GLTFNode() = default;
};

class GLTF
{
public:
    String Path;
    String Directory;

    GLTFNode* Root;
    Vector<GLTFMaterial> Materials;

    UInt32 VertexCount = 0;
    UInt32 IndexCount = 0;

    void Load(RHI::Ref rhi, const String& path);
    ~GLTF();

private:
    RHI::Ref mRHI;

    void ProcessPrimitive(cgltf_primitive *primitive, GLTFNode *node);
    void ProcessNode(cgltf_node *node, GLTFNode *mnode);
    void FreeNodes(GLTFNode* node);
};
