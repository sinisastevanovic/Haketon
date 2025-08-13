#include "Project.h"
#include "Haketon/Utils/PlatformUtils.h"
#include "Haketon/Core/Log.h"
#include "Haketon/Core/PathUtils.h"
#include <filesystem>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <cstdlib>
#include <algorithm>

namespace Haketon
{
    Project::Project(const std::string& projectPath)
        : m_ProjectDirectory(std::filesystem::path(projectPath).parent_path().string())
        , m_ProjectFilePath(projectPath)
    {
    }

    Ref<Project> Project::New(const std::string& projectPath, const std::string& name)
    {
        auto project = CreateRef<Project>(projectPath);
        project->m_Config.Name = name;
        
        // Create project directory structure
        std::filesystem::path projectDir(projectPath);
        projectDir = projectDir.parent_path();
        
        std::filesystem::create_directories(projectDir);
        std::filesystem::create_directories(projectDir / project->m_Config.AssetDirectory);
        std::filesystem::create_directories(projectDir / project->m_Config.AssetDirectory / "scenes");
        std::filesystem::create_directories(projectDir / project->m_Config.AssetDirectory / "textures");
        std::filesystem::create_directories(projectDir / project->m_Config.AssetDirectory / "shaders");
        std::filesystem::create_directories(projectDir / "src");
        std::filesystem::create_directories(projectDir / "tmp");
        std::filesystem::create_directories(projectDir / "vendor" / "premake");
        
        if (!project->GenerateGameTemplate())
        {
            HK_CORE_ERROR("Failed to generate game template");
            return nullptr;
        }
        
        if (!project->GeneratePremakeFile())
        {
            HK_CORE_ERROR("Failed to generate premake file");
            return nullptr;
        }
        
        if (!project->CopyPremakeFiles())
        {
            HK_CORE_ERROR("Failed to copy premake files");
            return nullptr;
        }
        
        if (!project->SaveProjectFile())
        {
            HK_CORE_ERROR("Failed to save project file");
            return nullptr;
        }
        
        // Set the game root path in PathUtils
        PathUtils::SetGameRootPath(project->m_ProjectDirectory);
        
        return project;
    }

    Ref<Project> Project::Load(const std::string& projectPath)
    {
        auto project = CreateRef<Project>(projectPath);
        
        if (!project->LoadProjectFile())
        {
            HK_CORE_ERROR("Failed to load project file: {0}", projectPath);
            return nullptr;
        }
        
        // Set the game root path in PathUtils
        PathUtils::SetGameRootPath(project->m_ProjectDirectory);
        
        return project;
    }

    bool Project::Save()
    {
        return SaveProjectFile();
    }

    bool Project::Build(const std::string& configuration)
    {
        HK_CORE_INFO("Building project: {0} in {1} configuration", m_Config.Name, configuration);
        
        if (!GenerateGameTemplate())
        {
            HK_CORE_ERROR("Failed to generate game template");
            return false;
        }
        
        return BuildWithMSBuild(configuration);
    }

    std::string Project::GetAssetDirectory() const
    {
        return (std::filesystem::path(m_ProjectDirectory) / m_Config.AssetDirectory).string();
    }

    std::string Project::GetOutputDirectory() const
    {
        return (std::filesystem::path(m_ProjectDirectory) / m_Config.OutputDirectory).string();
    }

    std::string Project::GetDllPath() const
    {
#ifdef HK_DEBUG
        std::filesystem::path dllPath = (std::filesystem::path(m_ProjectDirectory) / m_Config.OutputDirectory / "DebugEditor-windows-x86_64" / m_Config.Name);
#else
        std::filesystem::path dllPath = (std::filesystem::path(m_ProjectDirectory) / m_Config.OutputDirectory / "ReleaseEditor-windows-x86_64" / m_Config.Name);
#endif
        dllPath /= m_Config.Name + ".dll";
        return dllPath.string();
    }

    bool Project::SaveProjectFile()
    {
        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();
        
        doc.AddMember("name", rapidjson::Value(m_Config.Name.c_str(), allocator), allocator);
        doc.AddMember("version", rapidjson::Value(m_Config.Version.c_str(), allocator), allocator);
        doc.AddMember("startupScene", rapidjson::Value(m_Config.StartupScene.c_str(), allocator), allocator);
        doc.AddMember("assetDirectory", rapidjson::Value(m_Config.AssetDirectory.c_str(), allocator), allocator);
        doc.AddMember("outputDirectory", rapidjson::Value(m_Config.OutputDirectory.c_str(), allocator), allocator);
        doc.AddMember("intermediateDirectory", rapidjson::Value(m_Config.IntermediateDirectory.c_str(), allocator), allocator);
        doc.AddMember("enableVSync", m_Config.EnableVSync, allocator);
        doc.AddMember("fullscreen", m_Config.Fullscreen, allocator);
        doc.AddMember("windowWidth", m_Config.WindowWidth, allocator);
        doc.AddMember("windowHeight", m_Config.WindowHeight, allocator);
        
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        
        std::ofstream file(m_ProjectFilePath);
        if (!file.is_open())
        {
            HK_CORE_ERROR("Failed to open project file for writing: {0}", m_ProjectFilePath);
            return false;
        }
        
        file << buffer.GetString();
        return true;
    }

    bool Project::LoadProjectFile()
    {
        std::ifstream file(m_ProjectFilePath);
        if (!file.is_open())
        {
            HK_CORE_ERROR("Failed to open project file: {0}", m_ProjectFilePath);
            return false;
        }
        
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        
        rapidjson::Document doc;
        doc.Parse(content.c_str());
        
        if (doc.HasParseError())
        {
            HK_CORE_ERROR("Failed to parse project file: {0}", m_ProjectFilePath);
            return false;
        }
        
        if (doc.HasMember("name")) m_Config.Name = doc["name"].GetString();
        if (doc.HasMember("version")) m_Config.Version = doc["version"].GetString();
        if (doc.HasMember("startupScene")) m_Config.StartupScene = doc["startupScene"].GetString();
        if (doc.HasMember("assetDirectory")) m_Config.AssetDirectory = doc["assetDirectory"].GetString();
        if (doc.HasMember("outputDirectory")) m_Config.OutputDirectory = doc["outputDirectory"].GetString();
        if (doc.HasMember("intermediateDirectory")) m_Config.IntermediateDirectory = doc["intermediateDirectory"].GetString();
        if (doc.HasMember("enableVSync")) m_Config.EnableVSync = doc["enableVSync"].GetBool();
        if (doc.HasMember("fullscreen")) m_Config.Fullscreen = doc["fullscreen"].GetBool();
        if (doc.HasMember("windowWidth")) m_Config.WindowWidth = doc["windowWidth"].GetInt();
        if (doc.HasMember("windowHeight")) m_Config.WindowHeight = doc["windowHeight"].GetInt();
        
        return true;
    }

    bool Project::GenerateGameTemplate()
    {
        std::filesystem::path srcDir = std::filesystem::path(m_ProjectDirectory) / "src";
        
        // Generate main.cpp
        std::ofstream mainFile(srcDir / "main.cpp");
        if (!mainFile.is_open())
        {
            HK_CORE_ERROR("Failed to create main.cpp");
            return false;
        }
        
        mainFile << R"(#include <Haketon.h>
#include "GameLayer.h"
#include "GeneratedFiles/AutoReflection.gen.h"
#include "GeneratedFiles/)" << m_Config.Name << R"(ComponentSerialization.gen.h"
#include "Haketon/Core/IApplicationContext.h"

#ifdef _WIN32
    #ifdef GAME_DLL
        #define GAME_API __declspec(dllexport)
    #else
        #define GAME_API __declspec(dllimport)
    #endif
#else
    #define GAME_API
#endif

class GameModuleContext : public Haketon::IApplicationContext
{
    // Define some game specific data here
};

Haketon::IApplicationContext* InitializeGame(Haketon::Application* app)
{
    Haketon::Register)" << m_Config.Name << R"(Components();
    Haketon::RegisterAll)" << m_Config.Name << R"(Types();

    auto* gameLayer = new GameLayer();
#ifndef GAME_DLL
    app->PushLayer(gameLayer);
#endif
    auto* context = new GameModuleContext();
    context->CreatedLayers.push_back(gameLayer);

    return context;
}

void ShutdownGame()
{
    Haketon::Unregister)" << m_Config.Name << R"(Components();
}

#ifdef GAME_DLL
extern "C" {

    GAME_API Haketon::IApplicationContext* AttachGameToHost(Haketon::Application* hostApp)
    {
        return InitializeGame(hostApp);
    }

    GAME_API void DetachGameFromHost(Haketon::IApplicationContext* context)
    {
        ShutdownGame();
        delete context;
    }
}
#else
#include <Haketon/Core/EntryPoint.h>

class )" << m_Config.Name << R"(App : public Haketon::Application
{
public:
    )" << m_Config.Name << R"(App(Haketon::ApplicationCommandLineArgs args)
        : Application(")" << m_Config.Name << R"(", args, false)
    {
        InitializeGame(this);
    }

    ~)" << m_Config.Name << R"(App()
    {
        ShutdownGame();
    }
};

Haketon::Application* Haketon::CreateApplication(Haketon::ApplicationCommandLineArgs args)
{
    return new )" << m_Config.Name << R"(App(args);
}
#endif
)";

        // Generate GameLayer.h
        std::ofstream gameLayerH(srcDir / "GameLayer.h");
        if (!gameLayerH.is_open())
        {
            HK_CORE_ERROR("Failed to create GameLayer.h");
            return false;
        }
        
        gameLayerH << R"(#pragma once

#include <Haketon.h>

class GameLayer : public Haketon::Layer
{
public:
    GameLayer();
    virtual ~GameLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate(Haketon::Timestep ts) override;
    virtual void OnEvent(Haketon::Event& e) override;
};
)";

        // Generate GameLayer.cpp
        std::ofstream gameLayerCpp(srcDir / "GameLayer.cpp");
        if (!gameLayerCpp.is_open())
        {
            HK_CORE_ERROR("Failed to create GameLayer.cpp");
            return false;
        }
        
        gameLayerCpp << R"(#include "GameLayer.h"

GameLayer::GameLayer()
    : Layer("GameLayer")
{
}

void GameLayer::OnAttach()
{
    HK_PROFILE_FUNCTION();
}

void GameLayer::OnDetach()
{
    HK_PROFILE_FUNCTION();
}

void GameLayer::OnUpdate(Haketon::Timestep ts)
{
    HK_PROFILE_FUNCTION();

    if (Haketon::Application::Get().GetActiveScene()->IsPaused())
            return;
}

void GameLayer::OnEvent(Haketon::Event& e)
{
    // Handle events here
}
)";

        return true;
    }


    bool Project::GeneratePremakeFile()
    {
        std::filesystem::path projectDir(m_ProjectDirectory);
        std::ofstream premakeFile(projectDir / "premake5.lua");
        
        if (!premakeFile.is_open())
        {
            HK_CORE_ERROR("Failed to create premake5.lua");
            return false;
        }
        premakeFile << R"(-- Haketon Engine path - check environment variable first, then fallback to absolute path
haketonEnginePath = os.getenv("HAKETON_ENGINE_PATH") or ")" << GetHaketonEnginePath() << R"(/"
if not haketonEnginePath then
    error("HAKETON_ENGINE_PATH environment variable not set. Please point it to your Haketon Engine root directory.")
end

HAKETON_ENGINE_ROOT = haketonEnginePath
)";
        premakeFile << R"(workspace ")" << m_Config.Name << R"("
	architecture "x86_64"
	startproject ")" << m_Config.Name << R"("

	configurations
	{
		"Debug",
		"Release",
        "DebugEditor",
        "ReleaseEditor"
	}

	flags
	{
		"MultiProcessorCompile"
	}

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    include (path.join(HAKETON_ENGINE_ROOT, "Engine.lua"))

-- Game Project
project ")" << m_Config.Name << R"("
	location "."
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir (")" << m_Config.OutputDirectory << R"(/" .. outputdir .. "/%{prj.name}")
	objdir (")" << m_Config.IntermediateDirectory << R"(/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"src",
		path.join(HAKETON_ENGINE_ROOT, "Haketon/src"),
		path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor"),
		"%{IncludeDir.fmt}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.rttr}",
	}

	filter "system:windows"
		systemversion "latest"
        buildoptions { "/utf-8" }

	filter "configurations:Debug or configurations:Release or configurations:Dist"
        kind "ConsoleApp"
        links { "Haketon" }

	filter "configurations:DebugEditor or configurations:ReleaseEditor"
        kind "SharedLib"
        defines { "GAME_DLL", "HK_ENGINE_DLL_IMPORT", "FMT_SHARED", "RTTR_DLL", "HK_EDITOR" }
        links { "Haketon" }

    filter "configurations:DebugEditor"
		libdirs { path.join(LibraryDir.RTTRDllLib, "Debug") }
		links { "rttr_core_d" }

	filter "configurations:ReleaseEditor"
		libdirs { path.join(LibraryDir.RTTRDllLib, "Release") }
		links { "rttr_core" }

	filter "configurations:Debug or configurations:DebugEditor"
		defines "HK_DEBUG"
		runtime "Debug"
		symbols "on"
		prebuildcommands
		{
			("dotnet %s/HaketonHeaderTool/bin/Debug/net8.0/HaketonHeaderTool.dll %s )" << m_Config.Name << R"("):format(HAKETON_ENGINE_ROOT, _SCRIPT_DIR),
			"GenerateProjects.bat"
		}

	filter "configurations:Release  or configurations:ReleaseEditor"
		defines "HK_RELEASE"
		runtime "Release"
		optimize "on"
		prebuildcommands
		{
			("dotnet %s/HaketonHeaderTool/bin/Release/net8.0/HaketonHeaderTool.dll %s )" << m_Config.Name << R"("):format(HAKETON_ENGINE_ROOT, _SCRIPT_DIR),
			"GenerateProjects.bat"
		}
)";

        HK_CORE_INFO("Premake file generated at: {0}", (projectDir / "premake5.lua").string());
        return true;
    }

    bool Project::CopyPremakeFiles()
    {
        std::filesystem::path projectDir(m_ProjectDirectory);
        std::filesystem::path engineRootDir(PathUtils::GetEngineRootPath().parent_path());
        
        // Copy premake executable
        std::filesystem::path sourcePremakeExe = engineRootDir / "vendor" / "premake" / "bin" / "premake5.exe";
        std::filesystem::path destPremakeExe = projectDir / "vendor" / "premake" / "premake5.exe";
        
        std::error_code ec;
        if (std::filesystem::exists(sourcePremakeExe))
        {
            std::filesystem::copy_file(sourcePremakeExe, destPremakeExe, ec);
            if (ec)
            {
                HK_CORE_ERROR("Failed to copy premake5.exe: {0}", ec.message());
                return false;
            }
        }
        else
        {
            HK_CORE_WARN("Premake executable not found at: {0}", sourcePremakeExe.string());
        }
        
        // Copy premake5.lua from vendor/premake if it exists
        std::filesystem::path sourcePremakeLua = engineRootDir / "vendor" / "premake" / "premake5.lua";
        std::filesystem::path destPremakeLua = projectDir / "vendor" / "premake" / "premake5.lua";
        
        if (std::filesystem::exists(sourcePremakeLua))
        {
            std::filesystem::copy_file(sourcePremakeLua, destPremakeLua, ec);
            if (ec)
            {
                HK_CORE_ERROR("Failed to copy premake5.lua: {0}", ec.message());
                return false;
            }
        }

        // Also generate a generate project files script
        std::ofstream genScript(projectDir / "GenerateProjects.bat");
        if (genScript.is_open())
        {
            genScript << "@echo off\n";
            genScript << "echo Generating Visual Studio 2022 project files...\n";
            genScript << "call vendor\\premake\\premake5.exe vs2022\n";
            genScript << "pause\n";
        }
        
        HK_CORE_INFO("Premake files copied successfully");
        return true;
    }

    bool Project::BuildWithMSBuild(const std::string& configuration)
    {
        std::filesystem::path projectDir(m_ProjectDirectory);
        std::ofstream buildScript(projectDir / "build.bat");
        
        if (!buildScript.is_open())
        {
            HK_CORE_ERROR("Failed to create build script");
            return false;
        }
        
        buildScript << "@echo off\n";
        buildScript << "echo Building " << m_Config.Name << " in " << configuration << " configuration...\n";
        buildScript << "echo Generating Visual Studio project files...\n";
        buildScript << "call vendor\\premake\\premake5.exe vs2022\n";
        buildScript << "if %ERRORLEVEL% NEQ 0 (\n";
        buildScript << "    echo Failed to generate project files\n";
        buildScript << "    pause\n";
        buildScript << "    exit /b 1\n";
        buildScript << ")\n";
        buildScript << "echo Building with MSBuild...\n";
        buildScript << "msbuild " << m_Config.Name << ".sln /p:Configuration=" << configuration << " /p:Platform=x64\n";
        buildScript << "if %ERRORLEVEL% NEQ 0 (\n";
        buildScript << "    echo Build failed\n";
        buildScript << "    pause\n";
        buildScript << "    exit /b 1\n";
        buildScript << ")\n";
        buildScript << "echo Build completed successfully!\n";
        buildScript << "pause\n";
        
        // Also generate a generate project files script
        std::ofstream genScript(projectDir / "GenerateProjects.bat");
        if (genScript.is_open())
        {
            genScript << "@echo off\n";
            genScript << "echo Generating Visual Studio 2022 project files...\n";
            genScript << "call vendor\\premake\\premake5.exe vs2022\n";
            genScript << "pause\n";
        }
        
        HK_CORE_INFO("Build scripts generated at: {0}", projectDir.string());
        return true;
    }

    std::string Project::GetHaketonEnginePath() const
    {
        return PathUtils::NormalizePath(PathUtils::GetEngineRootPath().parent_path());
    }
}