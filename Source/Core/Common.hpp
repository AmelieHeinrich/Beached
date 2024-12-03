//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 05:31:48
//

#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>
#include <array>
#include <string>

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

template<typename T>
using Vector = std::vector<T>;

template<typename T, UInt64 Size>
using Array = std::array<T, Size>;

template<typename Key, typename Value>
using UnorderedMap = std::unordered_map<Key, Value>;

template<typename First, typename Second>
using Pair = std::pair<First, Second>;

using String = std::string;
using WideString = std::wstring;
