#include "NativeScriptComponentDetailCustomization.h"
#include <imgui/imgui.h>

namespace Haketon
{
    void NativeScriptComponentDetailCustomization::CustomizeContent(rttr::instance Instance)
    {
        if(!Instance.is_valid())
            return;
    
        Haketon::NativeScriptComponent* component = Instance.try_convert<Haketon::NativeScriptComponent>();
        if(component)
        {
            ImGui::Text("Script Class");
            ImGui::SameLine();
        
            if (DrawScriptSelector(component->ScriptClassName, *component))
            {
                component->UpdateBinding();
            }
        }
    }

    bool NativeScriptComponentDetailCustomization::DrawScriptSelector(std::string& scriptClassName, Haketon::NativeScriptComponent& component)
    {
        const auto& scripts = Haketon::ScriptRegistry::Get().GetRegisteredScripts();
    
        int currentItem = -1;
        std::vector<const char*> scriptNames;
        scriptNames.push_back("None");
    
        for (size_t i = 0; i < scripts.size(); ++i)
        {
            scriptNames.push_back(scripts[i].Name.c_str());
            if (scripts[i].Name == scriptClassName)
            {
                currentItem = static_cast<int>(i + 1);
            }
        }
    
        if (currentItem == -1)
        {
            currentItem = 0;
        }

        bool changed = false;
        if (ImGui::Combo("##ScriptClass", &currentItem, scriptNames.data(), static_cast<int>(scriptNames.size())))
        {
            if (currentItem == 0)
            {
                scriptClassName = "";
            }
            else
            {
                scriptClassName = scripts[currentItem - 1].Name;
            }
            changed = true;
        }

        return changed;
    }
}