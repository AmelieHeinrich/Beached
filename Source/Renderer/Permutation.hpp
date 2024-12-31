//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-31 08:36:39
//

#pragma once

#include <RHI/RHI.hpp>

class Permutation
{
public:
    Permutation() = default;
    Permutation(RHI::Ref rhi, GraphicsPipelineSpecs& specs);
    ~Permutation() = default;

    void Init(RHI::Ref rhi, GraphicsPipelineSpecs& specs);
    void AddPermutation(const String& name, const String& vertex, const String& fragment);
    GraphicsPipeline::Ref Get(const String& name) { return mPermutations[name]; }
private:
    GraphicsPipelineSpecs mSpecs;
    RHI::Ref mRHI;
    
    UnorderedMap<String, GraphicsPipeline::Ref> mPermutations;
};
