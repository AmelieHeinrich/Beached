//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 05:54:34
//

#include <Windows.h>
#include <sstream>

#include <Core/Assert.hpp>
#include <Core/Logger.hpp>

void Assert::Check(bool condition, const String& fileName, const String& function, int line, const String& message)
{
    if (!condition) {
        LOG_CRITICAL("ASSERTION FAILED ({0}:{1} - line {2}): {3}", fileName, function, line, message);
        MessageBoxA(nullptr, "Assertion Failed! Check output or log files. for details.", "BEACHED", MB_OK | MB_ICONERROR);
        __debugbreak();
    }
}
