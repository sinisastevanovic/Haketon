#pragma once

#include <string>
#include <vector>
#include "Haketon/Core/Core.h"

namespace Haketon
{
    struct ProjectConfig
    {
        std::string Name;
        std::string Version = "1.0.0";
        std::string StartupScene;
        std::string AssetDirectory = "assets";
        std::string OutputDirectory = "bin";
        std::string IntermediateDirectory = "temp";
        
        bool EnableVSync = true;
        bool Fullscreen = false;
        int WindowWidth = 1280;
        int WindowHeight = 720;
    };

    class Project
    {
    public:
        Project() = default;
        Project(const std::string& projectPath);
        
        static Ref<Project> New(const std::string& projectPath, const std::string& name);
        static Ref<Project> Load(const std::string& projectPath);
        
        bool Save();
        bool Build(const std::string& configuration = "Release");
        
        const ProjectConfig& GetConfig() const { return m_Config; }
        ProjectConfig& GetConfig() { return m_Config; }
        
        const std::string& GetProjectDirectory() const { return m_ProjectDirectory; }
        const std::string& GetProjectFilePath() const { return m_ProjectFilePath; }
        
        std::string GetAssetDirectory() const;
        std::string GetOutputDirectory() const;
        
    private:
        bool SaveProjectFile();
        bool LoadProjectFile();
        bool GenerateGameTemplate();
        bool GeneratePremakeFile();
        bool CopyPremakeFiles();
        bool BuildWithMSBuild(const std::string& configuration);
        std::string GetHaketonEnginePath() const;
        
    private:
        ProjectConfig m_Config;
        std::string m_ProjectDirectory;
        std::string m_ProjectFilePath;
    };
}