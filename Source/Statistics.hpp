//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-01 03:58:58
//

#pragma once

#include <Core/Common.hpp>

struct Statistics
{
    UInt64 InstanceCount = 0;
    UInt64 TriangleCount = 0;
    UInt64 CulledInstances = 0;
    UInt64 CulledTriangles = 0;
    UInt64 DispatchCount = 0;
    UInt64 DrawCallCount = 0;

    UInt64 UsedVRAM = 0;
    UInt64 MaxVRAM = 0;

    UInt64 UsedRAM = 0;
    UInt64 MaxRAM = 0;

    int Battery = 0;

    static void Reset()
    {
        Statistics& stats = Get();
        stats.DrawCallCount = 0;
        stats.InstanceCount = 0;
        stats.TriangleCount = 0;
        stats.DispatchCount = 0;
        stats.CulledInstances = 0;
        stats.CulledTriangles = 0;
    }

    static Statistics& Get()
    {
        static Statistics stats;
        return stats;
    }

    static void Update();
};
