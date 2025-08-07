#pragma once
#include "DetailCustomization/IDetailCustomization.h"
#include "Haketon/Scene/Components.h"
#include "Haketon/Scene/ScriptRegistry.h"
#include "Haketon/Scene/ScriptData.h"

namespace Haketon
{
    class NativeScriptComponentDetailCustomization : public IComponentContentCustomization
    {
    public:
        void CustomizeContent(rttr::instance Instance) override;

    private:
        bool DrawScriptSelector(std::string& scriptClassName, Haketon::NativeScriptComponent& component);
        void DrawDataProperties(NativeScriptComponent& component);
        bool DrawScriptDataWidget(ScriptData& scriptData);
        bool CreateValueWidgetFromVariant(rttr::variant& Value, const std::string& Type, bool bReadOnly = false);
    };
}
