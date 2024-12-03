//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 18:42:13
//

#pragma once

#include <Core/Common.hpp>

class File
{
public:
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
    static void *ReadBytes(const String& path);
};
