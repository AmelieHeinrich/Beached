//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-04 01:39:46
//

#pragma once

#define TO_SECONDS(Value) Value / 1000.0f

#include <Windows.h>

class Timer
{
public:
    Timer();

    float GetElapsed();
    void Restart();
private:
    LARGE_INTEGER mFrequency;
    LARGE_INTEGER mStart;
};
