//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-05 23:14:46
//

#pragma once

#include <Renderer/RenderPass.hpp>

class Renderer
{
public:
    using Ref = Ref<Renderer>;

    Renderer(RHI::Ref rhi);
    ~Renderer() = default;

    void Render(const Frame& frame, const Scene& scene);
    void UI();
private:
    Vector<RenderPass::Ref> mPasses;
};