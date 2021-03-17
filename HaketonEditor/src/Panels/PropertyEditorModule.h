#pragma once
#include "DetailCustomization/IDetailCustomization.h"
#include "Haketon/Core/Core.h"
#include "Haketon/Core/ModuleInterface.h"

class PropertyEditorModule : public IModuleInterface
{
public:

    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    void RegisterDetailCustomization(const std::string ClassName, std::function<Haketon::Ref<IDetailCustomization>()> CreateInstFunction);
    Haketon::Ref<IDetailCustomization> GetDetailCustomization(std::string ClassName);

private:

    std::unordered_map<std::string, std::function<Haketon::Ref<IDetailCustomization>()>> DetailCustomizationMap;
};
