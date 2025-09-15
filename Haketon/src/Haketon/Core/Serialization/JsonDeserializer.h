#pragma once

#include "ISerializer.h"
#include <nlohmann/json.hpp>

namespace Haketon
{
    class HK_API JsonDeserializer : public IDeserializerNew
    {
    public:

        JsonDeserializer();
        ~JsonDeserializer();

        bool DeserializeVar(const std::string& data, rttr::variant& object) override;

    private:
        struct Impl;
        std::unique_ptr<Impl> m_Impl;
    
    };
}

