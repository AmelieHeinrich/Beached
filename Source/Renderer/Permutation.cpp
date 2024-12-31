//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-31 08:55:09
//

#include <Renderer/Permutation.hpp>
#include <Asset/AssetManager.hpp>

Permutation::Permutation(RHI::Ref rhi, GraphicsPipelineSpecs& specs)
    : mRHI(rhi), mSpecs(specs)
{

}

void Permutation::Init(RHI::Ref rhi, GraphicsPipelineSpecs& specs)
{
    mRHI = rhi;
    mSpecs = specs;
}

void Permutation::AddPermutation(const String& name, const String& vertex, const String& fragment)
{
    Asset::Handle vertexShader = AssetManager::Get(vertex, AssetType::Shader);
    Asset::Handle pixelShader = AssetManager::Get(fragment, AssetType::Shader);

    mSpecs.Bytecodes[ShaderType::Vertex] = vertexShader->Shader;
    mSpecs.Bytecodes[ShaderType::Fragment] = pixelShader->Shader;
    mPermutations[name] = mRHI->CreateGraphicsPipeline(mSpecs);
}
