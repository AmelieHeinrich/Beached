//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 06:20:19
//

#pragma once

#include <string>

class UTF
{
public:
    static std::string WideToAscii(const wchar_t* text);
    static std::string WideToAscii(const std::wstring& text);
    static std::wstring AsciiToWide(const char* text);
    static std::wstring AsciiToWide(const std::string& text);
};
