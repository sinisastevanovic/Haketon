#pragma once
#include "DetailCustomization/IDetailCustomization.h"
#include "Haketon/Scene/Components.h"
#include "Haketon/Scene/ScriptRegistry.h"

namespace Haketon
{
    class NativeScriptComponentDetailCustomization : public IDetailCustomization
    {
    public:
        void CustomizeDetails(rttr::instance Instance) override;

    private:
        bool DrawScriptSelector(std::string& scriptClassName, Haketon::NativeScriptComponent& component);
    };
}
