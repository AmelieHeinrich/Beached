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

    const float vertices[] = {
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
    };

    const UInt32 indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    mVertexBuffer = mRHI->CreateBuffer(sizeof(vertices), sizeof(float) * 5, BufferType::Vertex, "Vertex Buffer");
    mIndexBuffer = mRHI->CreateBuffer(sizeof(indices), sizeof(UInt32), BufferType::Index, "Index Buffer");
    
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        mConstantBuffer[i] = mRHI->CreateBuffer(256, 0, BufferType::Constant, "Constant Buffer");
        mConstantBuffer[i]->BuildCBV();
    }

    Image image;
    image.Load("Assets/Textures/texture.jpg");

    TextureDesc desc = {};
    desc.Width = image.Width;
    desc.Height = image.Height;
    desc.Levels = 1;
    desc.Depth = 1;
    desc.Usage = TextureUsage::ShaderResource;
    desc.Name = "Albedo Texture";
    desc.Format = TextureFormat::RGBA8;
    mTexture = mRHI->CreateTexture(desc);
    mTextureView = mRHI->CreateView(mTexture, ViewType::ShaderResource);

    Uploader::EnqueueBufferUpload((void*)vertices, sizeof(vertices), mVertexBuffer);
    Uploader::EnqueueBufferUpload((void*)indices, sizeof(indices), mIndexBuffer);
    Uploader::EnqueueTextureUpload(image, mTexture);

    GraphicsPipelineSpecs triangleSpecs;
    triangleSpecs.Fill = FillMode::Solid;
    triangleSpecs.Cull = CullMode::None;
    triangleSpecs.Formats.push_back(TextureFormat::RGBA8);
    triangleSpecs.DepthEnabled = false;
    triangleSpecs.Bytecodes[ShaderType::Vertex] = ShaderCompiler::Compile("Assets/Shaders/Triangle/Vertex.hlsl", "VSMain", ShaderType::Vertex);
    triangleSpecs.Bytecodes[ShaderType::Fragment] = ShaderCompiler::Compile("Assets/Shaders/Triangle/Fragment.hlsl", "PSMain", ShaderType::Fragment);
    triangleSpecs.Signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(int) * 3);

    mSampler = mRHI->CreateSampler(SamplerAddress::Wrap, SamplerFilter::Linear);

    mPipeline = mRHI->CreateGraphicsPipeline(triangleSpecs);

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
        frame.CommandBuffer->ClearRenderTarget(frame.BackbufferView, 0.0f, 0.0f, 0.0f);
        frame.CommandBuffer->SetRenderTargets({ frame.BackbufferView }, nullptr);
       
        glm::mat4 uploads[] = {
            mCamera.View(),
            mCamera.Projection()
        };
        mConstantBuffer[frame.FrameIndex]->CopyMapped(uploads, sizeof(uploads));
        Int32 cbv = mConstantBuffer[frame.FrameIndex]->CBV();

        // Triangle
        int resources[] = {
            mConstantBuffer[frame.FrameIndex]->CBV(),
            mTextureView->GetDescriptor().Index,
            mSampler->BindlesssSampler()
        };

        frame.CommandBuffer->SetTopology(Topology::TriangleList);
        frame.CommandBuffer->SetViewport(0, 0, (float)width, (float)height);
        frame.CommandBuffer->SetGraphicsPipeline(mPipeline);
        frame.CommandBuffer->SetVertexBuffer(mVertexBuffer);
        frame.CommandBuffer->SetIndexBuffer(mIndexBuffer);
        frame.CommandBuffer->GraphicsPushConstants(resources, sizeof(int) * 3, 0);
        frame.CommandBuffer->DrawIndexed(6);

        // UI
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
