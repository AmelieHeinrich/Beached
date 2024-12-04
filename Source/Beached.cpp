//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 05:43:11
//

#include <Beached.hpp>

#include <Core/Logger.hpp>
#include <UI/Helpers.hpp>

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

Beached::Beached()
{
    Logger::Init();

    mWindow = MakeRef<Window>(1440, 900, "Beached");
    mRHI = MakeRef<RHI>(mWindow);
    AssetManager::Init(mRHI);

    mModel = AssetManager::Get("Assets/Models/Sponza/Sponza.gltf", ResourceType::GLTF);

    GraphicsPipelineSpecs triangleSpecs;
    triangleSpecs.Fill = FillMode::Solid;
    triangleSpecs.Cull = CullMode::None;
    triangleSpecs.Depth = DepthOperation::Less;
    triangleSpecs.CCW = false;
    triangleSpecs.DepthEnabled = true;
    triangleSpecs.DepthFormat = TextureFormat::Depth32;
    triangleSpecs.Formats.push_back(TextureFormat::RGBA8);
    triangleSpecs.DepthEnabled = true;
    triangleSpecs.Bytecodes[ShaderType::Vertex] = ShaderCompiler::Compile("Assets/Shaders/Triangle/Vertex.hlsl", "VSMain", ShaderType::Vertex);
    triangleSpecs.Bytecodes[ShaderType::Fragment] = ShaderCompiler::Compile("Assets/Shaders/Triangle/Fragment.hlsl", "PSMain", ShaderType::Fragment);
    triangleSpecs.Signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(int) * 4);

    mSampler = mRHI->CreateSampler(SamplerAddress::Wrap, SamplerFilter::Linear);
    mPipeline = mRHI->CreateGraphicsPipeline(triangleSpecs);

    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        mConstantBuffer[i] = mRHI->CreateBuffer(256, 0, BufferType::Constant, "CBV");
        mConstantBuffer[i]->BuildCBV();
    }

    int width, height;
    mWindow->PollSize(width, height);

    TextureDesc depthDesc;
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.Levels = 1;
    depthDesc.Format = TextureFormat::Depth32;
    depthDesc.Usage = TextureUsage::DepthTarget;
    depthDesc.Depth = 1;
    depthDesc.Name = "Depth Buffer";
    mDepth = mRHI->CreateTexture(depthDesc);
    mDepthView = mRHI->CreateView(mDepth, ViewType::DepthTarget);

    Uploader::Flush();
    mRHI->Wait();
    Uploader::ClearRequests();

    LOG_INFO("Starting Beached");
}

Beached::~Beached()
{

}

void Beached::Run()
{
    while (mWindow->IsOpen()) {
        float time = mTimer.GetElapsed();
        float dt = time - mLastFrame;
        mLastFrame = time;
        dt /= 1000.0f;
        
        mWindow->PollEvents();

        int width, height;
        mWindow->PollSize(width, height);

        mCamera.Begin();

        if (ImGui::IsKeyPressed(ImGuiKey_F1, false)) {
            mUI = !mUI;
        }

        Frame frame = mRHI->Begin();
        
        frame.CommandBuffer->Begin();
        frame.CommandBuffer->Barrier(frame.Backbuffer, ResourceLayout::ColorWrite);
        frame.CommandBuffer->Barrier(mDepth, ResourceLayout::DepthWrite);
        frame.CommandBuffer->ClearRenderTarget(frame.BackbufferView, 0.0f, 0.0f, 0.0f);
        frame.CommandBuffer->ClearDepth(mDepthView);
        frame.CommandBuffer->SetRenderTargets({ frame.BackbufferView }, mDepthView);
       
        glm::mat4 uploads[] = {
            mCamera.View(),
            mCamera.Projection()
        };
        mConstantBuffer[frame.FrameIndex]->CopyMapped(uploads, sizeof(uploads));

        // Renderer
        std::function<void(Frame frame, GLTFNode*, GLTF* model, glm::mat4 transform)> drawNode = [&](Frame frame, GLTFNode* node, GLTF* model, glm::mat4 transform) {
            if (!node->Children.empty()) {
                for (GLTFNode* child : node->Children) {
                    drawNode(frame, child, model, transform);
                }
            }

            for (GLTFPrimitive primitive : node->Primitives) {
                GLTFMaterial material = model->Materials[primitive.MaterialIndex];
                glm::mat4 globalTransform = transform * node->Transform * (node->Parent ? node->Parent->Transform : glm::mat4(1.0f));
            
                node->ModelBuffer[frame.FrameIndex]->CopyMapped(glm::value_ptr(globalTransform), sizeof(glm::mat4));

                int resources[] = {
                    mConstantBuffer[frame.FrameIndex]->CBV(),
                    node->ModelBuffer[frame.FrameIndex]->CBV(),
                    material.AlbedoView->GetDescriptor().Index,
                    mSampler->BindlesssSampler()
                };

                frame.CommandBuffer->GraphicsPushConstants(resources, sizeof(resources), 0);
                frame.CommandBuffer->SetVertexBuffer(primitive.VertexBuffer);
                frame.CommandBuffer->SetIndexBuffer(primitive.IndexBuffer);
                frame.CommandBuffer->DrawIndexed(primitive.IndexCount);
            }
        };

        frame.CommandBuffer->SetTopology(Topology::TriangleList);
        frame.CommandBuffer->SetViewport(0, 0, (float)width, (float)height);
        frame.CommandBuffer->SetGraphicsPipeline(mPipeline);
        drawNode(frame, mModel->Model.Root, &mModel->Model, glm::mat4(1.0f));

        // UI
        ImGuiIO& io = ImGui::GetIO();

        frame.CommandBuffer->BeginGUI(width, height);
        if (mUI) {
            UI();
        } else {
            Overlay();
        }
        frame.CommandBuffer->EndGUI();
        
        frame.CommandBuffer->Barrier(frame.Backbuffer, ResourceLayout::Present);
        frame.CommandBuffer->End();
        
        mRHI->Submit({ frame.CommandBuffer });
        mRHI->End();
        mRHI->Present(false);

        if (!io.WantCaptureMouse)
            mCamera.Update(dt, width, height);
    }
    mRHI->Wait();
}

void Beached::Overlay()
{
    UI::BeginCornerOverlay();
    ImGui::Text("Debug Menu: F1");
    ImGui::End();
}

void Beached::UI()
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Window")) {
            if (ImGui::MenuItem("Quit")) {
                // todo
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    UI::BeginCornerOverlay();
    ImGui::Text("Version 0.0.1");
    ImGui::Text("Renderer: Direct3D 12");
    ImGui::End();
}
