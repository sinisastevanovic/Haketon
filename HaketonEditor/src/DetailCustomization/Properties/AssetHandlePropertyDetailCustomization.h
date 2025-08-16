#pragma once
#include "DetailCustomization/IDetailCustomization.h"
#include "Haketon/Asset/AssetHandle.h"
#include "Haketon/Asset/AssetTypes.h"

namespace Haketon
{
    class AssetHandlePropertyDetailCustomization : public IPropertyDetailCustomization
    {
    public:
        bool CustomizeDetails(rttr::variant& Value, rttr::property& Property, bool bReadOnly = false) override;

    private:
        bool DrawAssetPicker(const std::string& label, AssetHandle& assetHandle, AssetType allowedType = AssetType::None);
        AssetType GetAssetTypeFromMetadata(rttr::property& property);
    };
}