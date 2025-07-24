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

    void RegisterPropertyDetailCustomization(const std::string ClassName, std::function<Haketon::Ref<IPropertyDetailCustomization>()> CreateInstFunction);
    Haketon::Ref<IPropertyDetailCustomization> GetPropertyDetailCustomization(std::string ClassName);

private:

    std::unordered_map<std::string, std::function<Haketon::Ref<IDetailCustomization>()>> DetailCustomizationMap;
    std::unordered_map<std::string, std::function<Haketon::Ref<IPropertyDetailCustomization>()>> PropertyDetailCustomizationMap;
};
