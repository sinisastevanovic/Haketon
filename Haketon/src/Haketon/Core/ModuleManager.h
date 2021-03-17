#pragma once

#include <string>
#include <unordered_map>

class IModuleInterface;

class ModuleManager
{
public:
    ModuleManager();
    ~ModuleManager();

    void AddModuleToList(std::string Name, IModuleInterface* Module);
    void StartupModule(std::string Name);

    static ModuleManager* Get() { return Instance; }

    template<typename TModuleInterface>
    static TModuleInterface* LoadModuleChecked(const std::string ModuleName)
    {
        IModuleInterface* ModuleInterface = ModuleManager::Get()->LoadModuleChecked(ModuleName);
        return (TModuleInterface*)(ModuleInterface);
    }

    IModuleInterface* LoadModuleChecked(const std::string ModuleName);

private:
    std::unordered_map<std::string, IModuleInterface*> ModulesMap;
    //std::vector<IModuleInterface*> m_Modules;

    static ModuleManager* Instance;
};
