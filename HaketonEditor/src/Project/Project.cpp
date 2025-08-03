#include "Project.h"
#include "Haketon/Utils/PlatformUtils.h"
#include "Haketon/Core/Log.h"
#include <filesystem>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

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
        
        if (!project->SaveProjectFile())
        {
            HK_CORE_ERROR("Failed to save project file");
            return nullptr;
        }
        
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

    bool Project::BuildWithMSBuild(const std::string& configuration)
    {
        // For now, just create a simple batch file to build the project
        // In a real implementation, you'd want to generate proper MSBuild files
        
        std::filesystem::path projectDir(m_ProjectDirectory);
        std::ofstream buildScript(projectDir / "build.bat");
        
        if (!buildScript.is_open())
        {
            HK_CORE_ERROR("Failed to create build script");
            return false;
        }
        
        buildScript << "@echo off\n";
        buildScript << "echo Building " << m_Config.Name << " in " << configuration << " configuration...\n";
        buildScript << "echo This is a placeholder build script.\n";
        buildScript << "echo In a real implementation, this would:\n";
        buildScript << "echo 1. Generate Visual Studio project files\n";
        buildScript << "echo 2. Build using MSBuild\n";
        buildScript << "echo 3. Copy assets to output directory\n";
        buildScript << "pause\n";
        
        HK_CORE_INFO("Build script generated at: {0}", (projectDir / "build.bat").string());
        HK_CORE_WARN("Project building is not fully implemented yet. A placeholder build script has been created.");
        
        return true;
    }
}