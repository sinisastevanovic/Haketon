#pragma once

#include "Core.h"
#include <string>
#include <filesystem>

namespace Haketon
{
    class HK_API PathUtils
    {
    public:
        static void Initialize();
        
        static std::filesystem::path GetExecutableDirectory();
        static std::filesystem::path GetWorkingDirectory();
        
        static std::filesystem::path GetEngineRootPath();
        static std::filesystem::path GetEngineAssetsPath();
        static std::filesystem::path GetEngineBinPath();
        
        static std::filesystem::path GetGameRootPath();
        static std::filesystem::path GetGameAssetsPath();
        static std::filesystem::path GetGameBinPath();
        static std::filesystem::path GetGameTmpPath();
        static std::filesystem::path GetGameCachePath();
        static std::filesystem::path GetPathRelativeToAssetsPath(const std::filesystem::path& absolutePath);
        
        static void SetGameRootPath(const std::filesystem::path& gamePath);
        
        static std::filesystem::path GetRelativePath(const std::filesystem::path& path, const std::filesystem::path& base);
        static std::filesystem::path GetAbsolutePath(const std::filesystem::path& relativePath, const std::filesystem::path& base);
        
        static bool EnsureDirectoryExists(const std::filesystem::path& path);
        static bool IsValidPath(const std::filesystem::path& path);
        
        static std::string NormalizePath(const std::filesystem::path& path);
        
    private:
        static std::filesystem::path s_EngineRootPath;
        static std::filesystem::path s_GameRootPath;
        static bool s_Initialized;
        
        static std::filesystem::path DetectEngineRoot();
    };
}