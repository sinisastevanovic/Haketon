#include "TagComponentDetailCustomization.h"

#include "Haketon/Scene/Components/TagComponent.h"

#include <imgui/imgui.h>

namespace Haketon
{
    void TagComponentDetailCustomization::CustomizeDetails(rttr::instance Instance)
    {
        if(!Instance.is_valid())
            return;
    
        Haketon::TagComponent* Component = Instance.try_convert<Haketon::TagComponent>();
        if(Component)
        {
            auto& tag = Component->Tag;

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strcpy_s(buffer, sizeof(buffer), tag.c_str());
            if(ImGui::InputText("##Tag", buffer, sizeof(buffer)))
            {
                tag = std::string(buffer);
            }
        }
    }
}

