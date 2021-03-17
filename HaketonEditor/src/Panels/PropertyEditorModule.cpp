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
