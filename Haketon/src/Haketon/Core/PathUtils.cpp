#include "hkpch.h"
#include "PathUtils.h"
#include "Log.h"

#ifdef HK_PLATFORM_WINDOWS
    #include <windows.h>
#endif

namespace Haketon
{
    std::filesystem::path PathUtils::s_EngineRootPath;
    std::filesystem::path PathUtils::s_GameRootPath;
    bool PathUtils::s_Initialized = false;

    void PathUtils::Initialize()
    {
        if (s_Initialized)
            return;

        s_EngineRootPath = DetectEngineRoot();
        HK_CORE_INFO("Engine root path: {0}", s_EngineRootPath.string());
        
        s_Initialized = true;
    }

    std::filesystem::path PathUtils::GetExecutableDirectory()
    {
#ifdef HK_PLATFORM_WINDOWS
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        std::filesystem::path exePath(buffer);
        return exePath.parent_path();
#else
        return std::filesystem::current_path();
#endif
    }

    std::filesystem::path PathUtils::GetWorkingDirectory()
    {
        return std::filesystem::current_path();
    }

    std::filesystem::path PathUtils::GetEngineRootPath()
    {
        if (!s_Initialized)
            Initialize();
        return s_EngineRootPath;
    }

    std::filesystem::path PathUtils::GetEngineAssetsPath()
    {
        return GetEngineRootPath() / "assets";
    }

    std::filesystem::path PathUtils::GetEngineBinPath()
    {
        return GetEngineRootPath() / "bin";
    }

    std::filesystem::path PathUtils::GetGameRootPath()
    {
        if (!s_Initialized)
            Initialize();
        return s_GameRootPath;
    }

    std::filesystem::path PathUtils::GetGameAssetsPath()
    {
        if (s_GameRootPath.empty())
            return std::filesystem::path();
        return s_GameRootPath / "assets";
    }

    std::filesystem::path PathUtils::GetGameBinPath()
    {
        if (s_GameRootPath.empty())
            return std::filesystem::path();
        return s_GameRootPath / "bin";
    }

    void PathUtils::SetGameRootPath(const std::filesystem::path& gamePath)
    {
        s_GameRootPath = std::filesystem::absolute(gamePath);
        HK_CORE_INFO("Game root path set to: {0}", s_GameRootPath.string());
    }

    std::filesystem::path PathUtils::GetRelativePath(const std::filesystem::path& path, const std::filesystem::path& base)
    {
        try
        {
            return std::filesystem::relative(path, base);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            HK_CORE_ERROR("Failed to get relative path: {0}", e.what());
            return path;
        }
    }

    std::filesystem::path PathUtils::GetAbsolutePath(const std::filesystem::path& relativePath, const std::filesystem::path& base)
    {
        try
        {
            return std::filesystem::absolute(base / relativePath);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            HK_CORE_ERROR("Failed to get absolute path: {0}", e.what());
            return relativePath;
        }
    }

    bool PathUtils::EnsureDirectoryExists(const std::filesystem::path& path)
    {
        try
        {
            if (!std::filesystem::exists(path))
            {
                return std::filesystem::create_directories(path);
            }
            return std::filesystem::is_directory(path);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            HK_CORE_ERROR("Failed to create directory {0}: {1}", path.string(), e.what());
            return false;
        }
    }

    bool PathUtils::IsValidPath(const std::filesystem::path& path)
    {
        try
        {
            return std::filesystem::exists(path);
        }
        catch (const std::filesystem::filesystem_error&)
        {
            return false;
        }
    }

    std::string PathUtils::NormalizePath(const std::filesystem::path& path)
    {
        std::string normalized = path.lexically_normal().string();
        std::replace(normalized.begin(), normalized.end(), '\\', '/');
        return normalized;
    }

    std::filesystem::path PathUtils::DetectEngineRoot()
    {
        std::filesystem::path current = GetExecutableDirectory();
        
        while (!current.empty() && current != current.parent_path())
        {
            if (std::filesystem::exists(current / "Haketon") && 
                std::filesystem::exists(current / "HaketonEditor") &&
                std::filesystem::exists(current / "premake5.lua"))
            {
                return current / "Haketon";
            }
            current = current.parent_path();
        }
        
        current = GetWorkingDirectory();
        while (!current.empty() && current != current.parent_path())
        {
            if (std::filesystem::exists(current / "Haketon") && 
                std::filesystem::exists(current / "HaketonEditor") &&
                std::filesystem::exists(current / "premake5.lua"))
            {
                return current;
            }
            current = current.parent_path();
        }
        
        HK_CORE_WARN("Could not detect engine root path, using working directory");
        return GetWorkingDirectory();
    }
}