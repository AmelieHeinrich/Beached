//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 18:42:13
//

#pragma once

#include <Core/Common.hpp>

class File
{
public:
    struct Filetime
    {
        UInt32 Low;
        UInt32 High;

        bool operator==(const Filetime& other) {
            return (Low == other.Low) && (High == other.High);
        }

        bool operator!=(const Filetime& other) {
            return !(*this == other);
        }
    };

    static bool Exists(const String& path);
    static bool IsDirectory(const String& path);
    
    static void CreateFileFromPath(const String& path);
    static void CreateDirectoryFromPath(const String& path);

    static void Delete(const String& path);
    static void Move(const String& oldPath, const String& newPath);
    static void Copy(const String& oldPath, const String& newPath, bool overwrite = true);

    static String GetFileExtension(const String& path);

    static int32_t GetFileSize(const String& path);
    static String ReadFile(const String& path);
    static void ReadBytes(const String& path, void *data, UInt64 size);
    static void *ReadBytes(const String& path);
    static void WriteBytes(const String& path, const void* data, UInt64 size);

    static Filetime GetLastModified(const String& path);
};
