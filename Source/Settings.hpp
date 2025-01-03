//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-01 03:29:32
//

#pragma once

#include <TOML++/toml.hpp>

struct Settings
{
    // Culling
    bool FrustumCull = true;
    bool FreezeFrustum = false;

    // Debug
    bool DebugDraw = true;
    bool DebugDrawLights = false;
    bool DebugDrawVolumes = false;
    
    // Composite
    float Gamma = 2.2f;

    static Settings& Get()
    {
        static Settings s;
        return s;
    };
};
