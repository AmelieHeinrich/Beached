//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 05:42:43
//

#pragma once

#include <Core/Window.hpp>
#include <Core/Timer.hpp>
#include <World/Camera.hpp>

#include <RHI/RHI.hpp>
#include <RHI/Uploader.hpp>
#include <RHI/Sampler.hpp>

class Beached
{
public:
    Beached();
    ~Beached();

    void Run();
private:
    void Overlay();
    void UI();

    Window::Ref mWindow;
    RHI::Ref mRHI;

    Camera mCamera;
    Timer mTimer;
    float mLastFrame;

    Buffer::Ref mVertexBuffer;
    Buffer::Ref mIndexBuffer;
    Array<Buffer::Ref, FRAMES_IN_FLIGHT> mConstantBuffer;
    Texture::Ref mTexture;
    View::Ref mTextureView;
    Sampler::Ref mSampler;
    GraphicsPipeline::Ref mPipeline;

    // UI settings
    bool mUI = false;
};
