//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 06:20:19
//

#pragma once

#include <Core/Common.hpp>

class UTF
{
public:
    static String WideToAscii(const wchar_t* text);
    static String WideToAscii(const WideString& text);
    static WideString AsciiToWide(const char* text);
    static WideString AsciiToWide(const String& text);
};
