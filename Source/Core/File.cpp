//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 18:42:51
//

#include <Core/File.hpp>
#include <Core/Logger.hpp>

#include <sys/stat.h>
#include <fstream>
#include <filesystem>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

bool File::Exists(const String& path)
{
    struct stat statistics;
    if (stat(path.c_str(), &statistics) == -1)
        return false;
    return true;
}

bool File::IsDirectory(const String& path)
{
    struct stat statistics;
    if (stat(path.c_str(), &statistics) == -1)
        return false;
    return (statistics.st_mode & S_IFDIR) != 0;
}

void File::CreateFileFromPath(const String& path)
{
    HANDLE handle = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (!handle) {
        LOG_ERROR("Error when creating file {0}", path.c_str());
        return;
    }
    CloseHandle(handle);
}

void File::CreateDirectoryFromPath(const String& path)
{
    if (!CreateDirectoryA(path.c_str(), nullptr)) {
        LOG_ERROR("Error when creating directory {0}", path.c_str());
    }
}

void File::Delete(const String& path)
{
    if (!Exists(path)) {
        LOG_WARN("Trying to delete file {0} that doesn't exist!", path.c_str());
        return;
    }

    if (!DeleteFileA(path.c_str())) {
        LOG_ERROR("Failed to delete file {0}", path.c_str());
    }
}

void File::Move(const String& oldPath, const String& newPath)
{
    if (!Exists(oldPath)) {
        LOG_WARN("Trying to move file {0} that doesn't exist!", oldPath.c_str());
        return;
    }

    if (!MoveFileA(oldPath.c_str(), newPath.c_str())) {
        LOG_ERROR("Failed to move file {0} to {1}", oldPath.c_str(), newPath.c_str());
    }
}

void File::Copy(const String& oldPath, const String& newPath, bool overwrite)
{
    if (!Exists(oldPath)) {
        LOG_WARN("Trying to copy file {0} that doesn't exist!", oldPath.c_str());
        return;
    }

    if (!CopyFileA(oldPath.c_str(), newPath.c_str(), !overwrite)) {
        LOG_ERROR("Failed to copy file {0} to {1}", oldPath.c_str(), newPath.c_str());
    }
}

String File::GetFileExtension(const String& path)
{
    std::filesystem::path fsPath(path);
    return fsPath.extension().string();
}

int File::GetFileSize(const String& path)
{
    int result = 0;

    HANDLE handle = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (!handle) {
        LOG_ERROR("File {0} does not exist!", path.c_str());
        return 0;
    }
    result = ::GetFileSize(handle, nullptr);
    CloseHandle(handle);
    return result;
}

String File::ReadFile(const String& path)
{
    HANDLE handle = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (!handle) {
        LOG_ERROR("File {0} does not exist and cannot be read!", path);
        return String("");
    }
    int size = File::GetFileSize(path);
    if (size == 0) {
        LOG_ERROR("File {0} has a size of 0, thus cannot be read!", path);
        return String("");
    }
    int bytesRead = 0;
    char *buffer = new char[size + 1];
    ::ReadFile(handle, reinterpret_cast<LPVOID>(buffer), size, reinterpret_cast<LPDWORD>(&bytesRead), nullptr);
    buffer[size] = '\0';
    CloseHandle(handle);
    return String(buffer);
}

void *File::ReadBytes(const String& path)
{
    HANDLE handle = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (!handle) {
        LOG_ERROR("File {0} does not exist and cannot be read!", path);
        return nullptr;
    }
    int size = File::GetFileSize(path);
    if (size == 0) {
        LOG_ERROR("File {0} has a size of 0, thus cannot be read!", path);
        return nullptr;
    }
    int bytesRead = 0;
    char *buffer = new char[size + 1];
    ::ReadFile(handle, reinterpret_cast<LPVOID>(buffer), size, reinterpret_cast<LPDWORD>(&bytesRead), nullptr);
    CloseHandle(handle);
    return buffer;
}