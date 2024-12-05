//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-06 00:09:15
//

#pragma once

#include <Asset/AssetManager.hpp>
#include <World/Camera.hpp>

struct Scene
{
    Camera Camera;
    Vector<Asset::Handle> Models;
};
