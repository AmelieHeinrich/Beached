//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 05:50:33
//

#pragma once

#include <spdlog/spdlog.h>

#include <Core/Common.hpp>

class Logger
{
public:
    static void Init();

    static Ref<spdlog::logger> GetLogger() { return sLogger; }
private:
    static Ref<spdlog::logger> sLogger;
};

#define LOG_TRACE(...)    ::Logger::GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)     ::Logger::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)     ::Logger::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    ::Logger::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::Logger::GetLogger()->critical(__VA_ARGS__)
