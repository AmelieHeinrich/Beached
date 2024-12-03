//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 05:31:48
//

#pragma once

#include <cstdint>
#include <memory>

typedef int16_t Int16;
typedef uint16_t UInt16;
typedef uint32_t UInt32;
typedef uint64_t UInt64;
typedef double Float64;

template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T, typename... Arguments>
constexpr Ref<T> MakeRef(Arguments&&... arguments)
{
    return std::make_shared<T>(std::forward<Arguments>(arguments)...);
}
