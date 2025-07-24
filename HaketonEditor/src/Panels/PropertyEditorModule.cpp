#include "hkpch.h"
#include "PropertyEditorModule.h"


void PropertyEditorModule::StartupModule()
{
}

void PropertyEditorModule::ShutdownModule()
{
}

void PropertyEditorModule::RegisterDetailCustomization(const std::string ClassName, std::function<Haketon::Ref<IDetailCustomization>()> CreateInstFunction)
{
    if(ClassName.length() > 0)
    {
        DetailCustomizationMap[ClassName] = CreateInstFunction;
    }
}

Haketon::Ref<IDetailCustomization> PropertyEditorModule::GetDetailCustomization(std::string ClassName)
{
    if(DetailCustomizationMap.find(ClassName) != DetailCustomizationMap.end())
    {
        return DetailCustomizationMap[ClassName]();
    }

    return nullptr;
}

void PropertyEditorModule::RegisterPropertyDetailCustomization(const std::string ClassName, std::function<Haketon::Ref<IPropertyDetailCustomization>()> CreateInstFunction)
{
    if(ClassName.length() > 0)
    {
        PropertyDetailCustomizationMap[ClassName] = CreateInstFunction;
    }
}

Haketon::Ref<IPropertyDetailCustomization> PropertyEditorModule::GetPropertyDetailCustomization(std::string ClassName)
{
    if (PropertyDetailCustomizationMap.find(ClassName) != PropertyDetailCustomizationMap.end())
    {
        return PropertyDetailCustomizationMap[ClassName]();
    }

    return nullptr;
}
