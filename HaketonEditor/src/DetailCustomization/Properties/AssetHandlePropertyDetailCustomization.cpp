#include "AssetHandlePropertyDetailCustomization.h"
#include <imgui.h>

#include "imgui_internal.h"
#include "Haketon/Core/Asset/AssetManager.h"
#include "Haketon/Core/Asset/AssetMetadata.h"

namespace Haketon
{
    bool AssetHandlePropertyDetailCustomization::CustomizeDetails(rttr::variant& Value, rttr::property& Property, bool bReadOnly)
    {
        if(bReadOnly)
        {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);  
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);
        }

        AssetHandle value = Value.get_value<AssetHandle>();
        AssetType allowedType = GetAssetTypeFromMetadata(Property);
        
        bool valueChanged = DrawAssetPicker("##", value, allowedType);
        
        if(valueChanged && !bReadOnly)
        {
            Value = value;
        }

        if(bReadOnly)
        {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }

        return valueChanged && !bReadOnly;
    }

    bool AssetHandlePropertyDetailCustomization::DrawAssetPicker(const std::string& label, AssetHandle& assetHandle, AssetType allowedType)
    {
        bool valueChanged = false;
        
        // Get the current asset name for display
        std::string currentAssetName = "None";
        if (assetHandle.IsValid())
        {
            const AssetMetadata* metadata = AssetManager::GetMetadata(assetHandle);
            if (metadata)
            {
                currentAssetName = metadata->SourceFilePath.filename().string();
            }
            else
            {
                currentAssetName = "Missing Asset";
            }
        }

        // Create the combo box
        if (ImGui::BeginCombo(label.c_str(), currentAssetName.c_str()))
        {
            // "None" option
            if (ImGui::Selectable("None", !assetHandle.IsValid()))
            {
                assetHandle = AssetHandle::Null();
                valueChanged = true;
            }

            // Get assets of the specified type
            std::vector<AssetMetadata> assets = AssetManager::GetAllAssetsOfTypeSorted(allowedType);
            
            for (const auto& metadata : assets)
            {
                bool isSelected = (assetHandle == metadata.Handle);
                if (ImGui::Selectable(metadata.SourceFilePath.filename().string().c_str(), isSelected))
                {
                    assetHandle = metadata.Handle;
                    valueChanged = true;
                }

                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        return valueChanged;
    }

    AssetType AssetHandlePropertyDetailCustomization::GetAssetTypeFromMetadata(rttr::property& property)
    {
        // Check for AssetType metadata
        rttr::variant assetTypeVariant = property.get_metadata("AssetType");
        if (assetTypeVariant.is_valid())
        {
            if (assetTypeVariant.can_convert<std::string>())
            {
                std::string typeStr = assetTypeVariant.get_value<std::string>();
                if (typeStr == "Texture") return AssetType::Texture;
                if (typeStr == "Mesh") return AssetType::Mesh;
                if (typeStr == "Material") return AssetType::Material;
                if (typeStr == "Scene") return AssetType::Scene;
            }
        }

        // Default to no filter (show all asset types)
        return AssetType::None;
    }
}