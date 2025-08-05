#pragma once
#include "DetailCustomization/IDetailCustomization.h"
#include "Haketon/Scene/Components.h"
#include "Haketon/Scene/ScriptRegistry.h"

namespace Haketon
{
    class NativeScriptComponentDetailCustomization : public IComponentContentCustomization
    {
    public:
        void CustomizeContent(rttr::instance Instance) override;

    private:
        bool DrawScriptSelector(std::string& scriptClassName, Haketon::NativeScriptComponent& component);
    };
}
