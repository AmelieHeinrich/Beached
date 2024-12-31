//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-04 03:43:25
//

#include <Asset/GLTF.hpp>
#include <Asset/AssetManager.hpp>
#include <Core/Assert.hpp>
#include <RHI/Uploader.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

void GLTF::Load(RHI::Ref rhi,const String& path)
{
    mRHI = rhi;
    Path = path;
    Directory = path.substr(0, path.find_last_of('/'));

    cgltf_options options = {};
    cgltf_data* data = nullptr;

    ASSERT(cgltf_parse_file(&options, path.c_str(), &data) == cgltf_result_success, "Failed to parse GLTF file!");
    ASSERT(cgltf_load_buffers(&options, data, path.c_str()) == cgltf_result_success, "Failed to load GLTF buffers!");
    cgltf_scene *scene = data->scene;

    Root = new GLTFNode;
    Root->Name = "RootNode";
    Root->Parent = nullptr;
    Root->Transform = glm::mat4(1.0f);
    Root->Children.resize(scene->nodes_count);

    for (int i = 0; i < scene->nodes_count; i++) {
        Root->Children[i] = new GLTFNode;
        Root->Children[i]->Parent = Root;
        Root->Children[i]->Transform = glm::mat4(1.0f);

        ProcessNode(scene->nodes[i], Root->Children[i]);
    }
}

GLTF::~GLTF()
{
    FreeNodes(Root);
    Materials.clear();
}

void GLTF::FreeNodes(GLTFNode* node)
{
    if (!node)
        return;

    for (GLTFNode* child : node->Children) {
        FreeNodes(child);
    }
    node->Children.clear();

    delete node;
}

void GLTF::ProcessNode(cgltf_node *node, GLTFNode *mnode)
{
    glm::mat4 localTransform(1.0f);
    glm::mat4 translationMatrix(1.0f);
    glm::mat4 rotationMatrix(1.0f);
    glm::mat4 scaleMatrix(1.0f);

    if (node->has_translation) {
        glm::vec3 translation = glm::vec3(node->translation[0], node->translation[1], node->translation[2]);
        translationMatrix = glm::translate(glm::mat4(1.0f), translation);
    }
    if (node->has_rotation) {
        rotationMatrix = glm::mat4_cast(glm::quat(node->rotation[3], node->rotation[0], node->rotation[1], node->rotation[2]));
    }
    if (node->has_scale) {
        glm::vec3 scale = glm::vec3(node->scale[0], node->scale[1], node->scale[2]);
        scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
    }

    if (node->has_matrix) {
        localTransform *= glm::make_mat4(node->matrix);
    } else {
        localTransform *= translationMatrix * rotationMatrix * scaleMatrix;
    }

    mnode->Name = node->name ? node->name : "Unnamed Node " + std::to_string(rand());
    mnode->Transform = localTransform;

    if (node->mesh) {
        for (int i = 0; i < node->mesh->primitives_count; i++) {
            ProcessPrimitive(&node->mesh->primitives[i], mnode);
        }
    }

    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        mnode->ModelBuffer[i] = mRHI->CreateBuffer(512, 0, BufferType::Constant, mnode->Name + std::string(" CBV") + std::to_string(i));
        mnode->ModelBuffer[i]->BuildCBV();
    }

    mnode->Children.resize(node->children_count);
    for (int i = 0; i < node->children_count; i++) {
        mnode->Children[i] = new GLTFNode;
        mnode->Children[i]->Parent = mnode;

        ProcessNode(node->children[i], mnode->Children[i]);
    }
}

void GLTF::ProcessPrimitive(cgltf_primitive *primitive, GLTFNode *node)
{
    if (primitive->type != cgltf_primitive_type_triangles) {
        return;
    }

    GLTFPrimitive out;

    cgltf_attribute* posAttribute = nullptr;
    cgltf_attribute* uvAttribute = nullptr;
    cgltf_attribute* normAttribute = nullptr;

    for (int i = 0; i < primitive->attributes_count; i++) {
        if (!strcmp(primitive->attributes[i].name, "POSITION")) {
            posAttribute = &primitive->attributes[i];
        }
        if (!strcmp(primitive->attributes[i].name, "TEXCOORD_0")) {
            uvAttribute = &primitive->attributes[i];
        }
        if (!strcmp(primitive->attributes[i].name, "NORMAL")) {
            normAttribute = &primitive->attributes[i];
        }
    }

    int vertexCount = posAttribute->data->count;
    int indexCount = primitive->indices->count;

    std::vector<Vertex> vertices = {};
    std::vector<UInt32> indices = {};

    for (int i = 0; i < vertexCount; i++) {
        Vertex vertex = {};

        //u32 ids[4];

        if (!cgltf_accessor_read_float(posAttribute->data, i, glm::value_ptr(vertex.Position), 4)) {
        }
        if (!cgltf_accessor_read_float(uvAttribute->data, i, glm::value_ptr(vertex.UV), 4)) {
        }
        if (!cgltf_accessor_read_float(normAttribute->data, i, glm::value_ptr(vertex.Normal), 4)) {
        }
        //if (!cgltf_accessor_read_uint(joint_attribute->data, i, ids, 4)) {
        //}
        //if (!cgltf_accessor_read_float(weight_attribute->data, i, vertex.Weights, 4)) {
        //}

        //for (u32 i = 0; i < MAX_BONE_WEIGHTS; i++)
        //    vertex.MaxBoneInfluence[i] = static_cast<int>(ids[i]);

        vertices.push_back(vertex);
    }

    for (int i = 0; i < indexCount; i++) {
        indices.push_back(cgltf_accessor_read_index(primitive->indices, i));
    }

    out.VertexCount = vertexCount;
    out.IndexCount = indexCount;

    /// @note(ame): create buffers
    out.VertexBuffer = mRHI->CreateBuffer(vertices.size() * sizeof(Vertex), sizeof(Vertex), BufferType::Vertex, node->Name + " Vertex Buffer");
    out.IndexBuffer = mRHI->CreateBuffer(indices.size() * sizeof(UInt32), sizeof(UInt32), BufferType::Index, node->Name + " Index Buffer");
    out.GeometryStructure = mRHI->CreateBLAS(out.VertexBuffer, out.IndexBuffer, out.VertexCount, out.IndexCount, node->Name + " BLAS");

    Uploader::EnqueueBufferUpload(vertices.data(), out.VertexBuffer->GetSize(), out.VertexBuffer);
    Uploader::EnqueueBufferUpload(indices.data(), out.IndexBuffer->GetSize(), out.IndexBuffer);

    out.Instance = {};
    out.Instance.AccelerationStructure = out.GeometryStructure->GetAddress();
    out.Instance.InstanceMask = 1;
    out.Instance.InstanceID = 0;
    out.Instance.Transform = glm::mat3x4(glm::transpose(node->Transform));
    out.Instance.Flags = 0x4;

    cgltf_material *material = primitive->material;
    GLTFMaterial outMaterial = {};
    out.MaterialIndex = Materials.size();
    outMaterial.AlphaTested = material->alpha_mode == cgltf_alpha_mode_mask;
    outMaterial.AlphaCutoff = material->alpha_cutoff;

    if (material && material->pbr_metallic_roughness.base_color_texture.texture) {
        std::string path = Directory + '/' + std::string(material->pbr_metallic_roughness.base_color_texture.texture->image->uri);
    
        outMaterial.Albedo = AssetManager::Get(path, AssetType::Texture);
        outMaterial.AlbedoView = mRHI->CreateView(outMaterial.Albedo->Texture, ViewType::ShaderResource);
    }
    if (material && material->normal_texture.texture) {
        std::string path = Directory + '/' + std::string(material->normal_texture.texture->image->uri);
    
        outMaterial.Normal = AssetManager::Get(path, AssetType::Texture);
        outMaterial.NormalView = mRHI->CreateView(outMaterial.Normal->Texture, ViewType::ShaderResource);
    }

    VertexCount += out.VertexCount;
    IndexCount += out.IndexCount;

    Materials.push_back(outMaterial);
    node->Primitives.push_back(out);
}

void GLTF::TraverseNode(GLTFNode* root, const std::function<void(GLTFNode*)>& fn)
{
    if (!root) {
        return;
    }
    fn(root);

    if (!root->Children.empty()) {
        for (GLTFNode* child : root->Children) {
            TraverseNode(child, fn);
        }
    }
}
