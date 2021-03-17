#include "hkpch.h"
#include "ModuleManager.h"

#include "ModuleInterface.h"

ModuleManager* ModuleManager::Instance = nullptr;

ModuleManager::ModuleManager()
{
    HK_CORE_ASSERT(!Instance, "Module Manager Instance already exists!");
    Instance = this;
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
