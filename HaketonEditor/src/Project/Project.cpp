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
#include <Haketon/Core/EntryPoint.h>
#include "GameLayer.h"

class )" << m_Config.Name << R"(App : public Haketon::Application
{
public:
    )" << m_Config.Name << R"(App(Haketon::ApplicationCommandLineArgs args)
        : Application(")" << m_Config.Name << R"(", args, false)
    {
        // Initialize your game here
        PushLayer(new GameLayer());
    }

    ~)" << m_Config.Name << R"(App()
    {
    }
};

namespace Haketon
{
    Application* CreateApplication(ApplicationCommandLineArgs args)
    {
        return new )" << m_Config.Name << R"(App(args);
    }
}
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

private:
    Haketon::Ref<Haketon::Scene> m_Scene;
    Haketon::Ref<Haketon::Framebuffer> m_Framebuffer;
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
#include "Haketon/Core/Serializer.h"

GameLayer::GameLayer()
    : Layer("GameLayer")
{
}

void GameLayer::OnAttach()
{
    HK_PROFILE_FUNCTION();
    
    Haketon::FramebufferSpecification fbSpec;
    fbSpec.Attachments = { Haketon::FramebufferTextureFormat::RGBA8, Haketon::FramebufferTextureFormat::Depth };
    fbSpec.Width = )" << m_Config.WindowWidth << R"(;
    fbSpec.Height = )" << m_Config.WindowHeight << R"(;
    m_Framebuffer = Haketon::Framebuffer::Create(fbSpec);

    m_Scene = Haketon::CreateRef<Haketon::Scene>();
    
    // Load startup scene if specified
)" << (m_Config.StartupScene.empty() ? "" : 
    "    if (!\"" + m_Config.StartupScene + "\".empty())\n"
    "    {\n"
    "        std::string scenePath = \"" + m_Config.AssetDirectory + "/scenes/" + m_Config.StartupScene + "\";\n"
    "        Haketon::Serializer::DeserializeSceneFromFile(scenePath, m_Scene);\n"
    "    }\n") << R"(
    
    m_Scene->OnViewportResize()" << m_Config.WindowWidth << ", " << m_Config.WindowHeight << R"();
}

void GameLayer::OnDetach()
{
    HK_PROFILE_FUNCTION();
}

void GameLayer::OnUpdate(Haketon::Timestep ts)
{
    HK_PROFILE_FUNCTION();
    
    // Update scene
    m_Scene->OnUpdateRuntime(ts);
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
HaketonPath = os.getenv("HAKETON_ENGINE_PATH") or ")" << GetHaketonEnginePath() << R"(/"
include (HaketonPath .. "Dependencies.lua")

)";
        premakeFile << R"(workspace ")" << m_Config.Name << R"("
	architecture "x86_64"
	startproject ")" << m_Config.Name << R"("

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Reference existing engine projects without regenerating them
externalproject "Haketon"
	location (HaketonPath .. "Haketon")
	kind "StaticLib"
	language "C++"

externalproject "HaketonEditor"
	location (HaketonPath .. "HaketonEditor")
	kind "ConsoleApp"
	language "C++"

-- Dependencies (still need to include these for the game project)
group "Dependencies"
	include (HaketonPath .. "vendor/premake")
	include (HaketonPath .. "Haketon/vendor/GLFW")
	include (HaketonPath .. "Haketon/vendor/Glad")
	include (HaketonPath .. "Haketon/vendor/imgui")
group ""

-- Game Project
project ")" << m_Config.Name << R"("
	location "."
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
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
		HaketonPath .. "Haketon/vendor/spdlog/include",
		HaketonPath .. "Haketon/src",
		HaketonPath .. "Haketon/vendor",
		HaketonPath .. "Haketon/vendor/glm",
        HaketonPath .. "Haketon/vendor/entt/include",
		HaketonPath .. "Haketon/vendor/rttr/include",
	}

	links
	{
		"Haketon"
	}

	filter "system:windows"
		systemversion "latest"
        buildoptions { "/utf-8" }

	filter "configurations:Debug"
		defines "HK_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "HK_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "HK_DIST"
		runtime "Release"
		optimize "on"
)";

        HK_CORE_INFO("Premake file generated at: {0}", (projectDir / "premake5.lua").string());
        return true;
    }

    bool Project::CopyPremakeFiles()
    {
        std::filesystem::path projectDir(m_ProjectDirectory);
        std::filesystem::path engineRootDir(GetHaketonEnginePath());
        
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
        return PathUtils::NormalizePath(PathUtils::GetEngineRootPath());
    }
}