//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-04 03:38:12
//

#pragma once

#include <Core/Common.hpp>
#include <RHI/RHI.hpp>
#include <RHI/BLAS.hpp>
#include <RHI/TLAS.hpp>
#include <Physics/Volume.hpp>

#include <cgltf/cgltf.h>
#include <glm/glm.hpp>
#include <functional>

class Asset;

struct Vertex
{
    glm::vec3 Position;
    glm::vec2 UV;
    glm::vec3 Normal;
};

struct GLTFMaterial
{
    Ref<Asset> Albedo;
    View::Ref AlbedoView;

    Ref<Asset> Normal;
    View::Ref NormalView;

    bool AlphaTested;
    float AlphaCutoff;

    glm::vec3 MaterialColor;
};

struct GLTFPrimitive
{
    Buffer::Ref VertexBuffer;
    Buffer::Ref IndexBuffer;

    RaytracingInstance Instance;
    BLAS::Ref GeometryStructure;

    UInt32 VertexCount;
    UInt32 IndexCount;
    int MaterialIndex;

    Box AABB;
};

struct GLTFNode
{
    Vector<GLTFPrimitive> Primitives;
    Array<Buffer::Ref, FRAMES_IN_FLIGHT> ModelBuffer;

    String Name = "";
    glm::mat4 Transform;
    GLTFNode* Parent = nullptr;
    Vector<GLTFNode*> Children = {};

    GLTFNode() = default;
};

class GLTF
{
public:
    String Path;
    String Directory;

    GLTFNode* Root = nullptr;
    Vector<GLTFMaterial> Materials;

    UInt32 VertexCount = 0;
    UInt32 IndexCount = 0;

    void Load(RHI::Ref rhi, const String& path);
    ~GLTF();

    void TraverseNode(GLTFNode* root, const std::function<void(GLTFNode*)>& fn);
private:
    RHI::Ref mRHI;

    void ProcessPrimitive(cgltf_primitive *primitive, GLTFNode *node);
    void ProcessNode(cgltf_node *node, GLTFNode *mnode);
    void FreeNodes(GLTFNode* node);
};
