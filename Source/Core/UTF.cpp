//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 06:20:56
//

#include <Core/UTF.hpp>

#include <codecvt>

std::string UTF::WideToAscii(const wchar_t* psText)
{
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;
    return converter.to_bytes(psText);
}

std::string UTF::WideToAscii(const std::wstring& sText)
{
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;
    return converter.to_bytes(sText.data());
}

std::wstring UTF::AsciiToWide(const char* sText)
{
    std::string_view Wrapped(sText);
    return std::wstring(Wrapped.begin(), Wrapped.end());
}

std::wstring UTF::AsciiToWide(const std::string& sText)
{
    return std::wstring(sText.begin(), sText.end());
}
