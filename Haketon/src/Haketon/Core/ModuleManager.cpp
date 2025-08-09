#include "hkpch.h"
#include "ModuleManager.h"

#include "ModuleInterface.h"

ModuleManager* ModuleManager::s_Instance = nullptr;

void ModuleManager::Initialize()
{
    if (!s_Instance)
    {
        s_Instance = new ModuleManager();
    }
}

void ModuleManager::Shutdown()
{
    if (s_Instance)
    {
        delete s_Instance;
        s_Instance = nullptr;
    }
}

ModuleManager& ModuleManager::Get()
{
    HK_CORE_ASSERT(s_Instance, "ModuleManager not initialized!");
    return *s_Instance;
}

ModuleManager::~ModuleManager()
{
    for(auto it = ModulesMap.cbegin(); it != ModulesMap.cend();)
    {
        ModulesMap.erase(it++);
    }
}

void ModuleManager::AddModuleToList(std::string Name, IModuleInterface* Module)
{
    ModulesMap[Name] = Module;
}

void ModuleManager::StartupModule(std::string Name)
{    
    if(ModulesMap.find(Name) != ModulesMap.end())
    {
        ModulesMap[Name]->StartupModule();
    }   
}

IModuleInterface* ModuleManager::LoadModuleChecked(const std::string ModuleName)
{
    IModuleInterface* Module = nullptr;
    if(ModulesMap.find(ModuleName) != ModulesMap.end())
    {
        Module = ModulesMap[ModuleName];
    }

    HK_CORE_ASSERT(Module, "Module could not be loaded!");
    return Module;
}
