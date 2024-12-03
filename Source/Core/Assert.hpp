//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 05:53:03
//

#pragma once

#include <Core/Common.hpp>
#include <string>

class Assert
{
public:
    static void Check(bool condition, const std::string& fileName, const std::string& function, int line, const std::string& message);
};

#define ASSERT(cond, msg) ::Assert::Check(cond, __FILE__, __FUNCTION__, __LINE__, msg)
