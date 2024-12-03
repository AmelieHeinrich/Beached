//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 19:17:51
//

#include <RHI/GraphicsPipeline.hpp>
#include <RHI/Utilities.hpp>
#include <Core/Assert.hpp>
#include <Core/Logger.hpp>

GraphicsPipeline::GraphicsPipeline(Device::Ref device, GraphicsPipelineSpecs& specs)
{
    Shader& vertexBytecode = specs.Bytecodes[ShaderType::Vertex];
    Shader& fragmentBytecode = specs.Bytecodes[ShaderType::Fragment];

    std::vector<D3D12_INPUT_ELEMENT_DESC> InputElementDescs;
    std::vector<std::string> InputElementSemanticNames;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc = {};
    Desc.VS.pShaderBytecode = vertexBytecode.Bytecode.data();
    Desc.VS.BytecodeLength = vertexBytecode.Bytecode.size();
    Desc.PS.pShaderBytecode = fragmentBytecode.Bytecode.data();
    Desc.PS.BytecodeLength = fragmentBytecode.Bytecode.size();
    for (int RTVIndex = 0; RTVIndex < specs.Formats.size(); RTVIndex++) {
        Desc.BlendState.RenderTarget[RTVIndex].SrcBlend = D3D12_BLEND_ONE;
        Desc.BlendState.RenderTarget[RTVIndex].DestBlend = D3D12_BLEND_ZERO;
        Desc.BlendState.RenderTarget[RTVIndex].BlendOp = D3D12_BLEND_OP_ADD;
        Desc.BlendState.RenderTarget[RTVIndex].SrcBlendAlpha = D3D12_BLEND_ONE;
        Desc.BlendState.RenderTarget[RTVIndex].DestBlendAlpha = D3D12_BLEND_ZERO;
        Desc.BlendState.RenderTarget[RTVIndex].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        Desc.BlendState.RenderTarget[RTVIndex].LogicOp = D3D12_LOGIC_OP_NOOP;
        Desc.BlendState.RenderTarget[RTVIndex].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        Desc.RTVFormats[RTVIndex] = DXGI_FORMAT(specs.Formats[RTVIndex]);
        Desc.NumRenderTargets = specs.Formats.size();
    }
    Desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    Desc.RasterizerState.FillMode = D3D12_FILL_MODE(specs.Fill);
    Desc.RasterizerState.CullMode = D3D12_CULL_MODE(specs.Cull);
    Desc.RasterizerState.DepthClipEnable = specs.DepthClipEnable;
    Desc.RasterizerState.FrontCounterClockwise = specs.CCW;
    Desc.PrimitiveTopologyType = specs.Line ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE : D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    if (specs.DepthEnabled) {
        Desc.DepthStencilState.DepthEnable = true;
        Desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        Desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC(specs.Depth);
        Desc.DSVFormat = DXGI_FORMAT(specs.DepthFormat);
    }
    Desc.SampleDesc.Count = 1;

    if (specs.Signature) {
        Desc.pRootSignature = specs.Signature->GetSignature();
        mSignature = specs.Signature;
    }

    HRESULT result = device->GetDevice()->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&mPipeline));
    ASSERT(SUCCEEDED(result), "Failed to create graphics pipeline!");
}

GraphicsPipeline::~GraphicsPipeline()
{
    D3DUtils::Release(mPipeline);
}
