#pragma once

#include "Haketon/Core/Core.h"
#include <string>
#include <unordered_map>

class IModuleInterface;

class HK_API ModuleManager
{
public:
    static void Initialize();
    static void Shutdown();
    static ModuleManager& Get();

    void AddModuleToList(std::string Name, IModuleInterface* Module);
    void StartupModule(std::string Name);

    template<typename TModuleInterface>
    static TModuleInterface* LoadModuleChecked(const std::string ModuleName)
    {
        IModuleInterface* ModuleInterface = ModuleManager::Get().LoadModuleChecked(ModuleName);
        return (TModuleInterface*)(ModuleInterface);
    }

    IModuleInterface* LoadModuleChecked(const std::string ModuleName);

private:
    ModuleManager() = default;
    ~ModuleManager();

    std::unordered_map<std::string, IModuleInterface*> ModulesMap;
    
    static ModuleManager* s_Instance;
};
