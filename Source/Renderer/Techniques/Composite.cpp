//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-06 00:22:01
//

#include <Renderer/Techniques/Composite.hpp>

Composite::Composite(RHI::Ref rhi)
    : RenderPass(rhi)
{
}

void Composite::Render(const Frame& frame, const Scene& scene)
{
    ::Ref<RenderPassIO> color = PassManager::Get("MainColorBuffer");

    frame.CommandBuffer->Barrier(color->Texture, ResourceLayout::CopySource);
    frame.CommandBuffer->Barrier(frame.Backbuffer, ResourceLayout::CopyDest);
    frame.CommandBuffer->CopyTextureToTexture(frame.Backbuffer, color->Texture);
}

void Composite::UI()
{

}
