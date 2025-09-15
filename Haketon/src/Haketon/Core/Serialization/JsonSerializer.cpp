#include "hkpch.h"
#include "JsonSerializer.h"
#include "TypeHandlerRegistry.h"

#include <rttr/registration>
#include <rttr/type>
#include <rttr/instance.h>
#include <map>

using json = nlohmann::json;

namespace Haketon
{
    struct JsonSerializer::Impl
    {
        uint64_t nextPointerId = 1;
        std::map<const void*, uint64_t> pointerIdMap;

        void Reset()
        {
            nextPointerId = 1;
            pointerIdMap.clear();
        }

        json SerializeClass(const rttr::variant& var);
        json ToJson(const rttr::variant& var);
    };

    JsonSerializer::JsonSerializer() : m_Impl(std::make_unique<Impl>()) {}
    JsonSerializer::~JsonSerializer() = default;

    std::string JsonSerializer::Serialize(const rttr::variant& object)
    {
        m_Impl->Reset();
        json jsonData = m_Impl->ToJson(object);
        return jsonData.dump(4);
    }

    json JsonSerializer::Impl::SerializeClass(const rttr::variant& var)
    {
        json j = json::object();
        rttr::type type = var.get_type();

        for (auto& prop : type.get_properties())
        {
            rttr::variant propValue = prop.get_value(var);
            if (propValue.is_valid())
            {
                j[prop.get_name().to_string()] = ToJson(propValue);
            }
        }
        return j;
    }

    json JsonSerializer::Impl::ToJson(const rttr::variant& var)
    {
        if (!var.is_valid())
            return nullptr;

        rttr::type type = var.get_type();
        rttr::type lookupType = type;
        if (type.is_wrapper())
        {
            lookupType = type.get_wrapped_type();
        }
        if (lookupType.is_pointer())
        {
            lookupType = lookupType.get_raw_type();
        }

        if (auto customSerializer = TypeHandlerRegistry::GetInstance().FindSerializer(lookupType))
        {
            return customSerializer(var);
        }

        if (type.is_pointer() || type.is_wrapper())
        {
            auto ptr = var.get_value<void*>();
            if (!ptr)
                return nullptr;

            if (pointerIdMap.count(ptr))
            {
                return json{{"$ref", pointerIdMap.at(ptr)}};
            }

            uint64_t newId = nextPointerId++;
            pointerIdMap[ptr] = newId;

            rttr::variant wrappedVar = var.extract_wrapped_value();
            if (!wrappedVar.is_valid())
                return nullptr;
            
            rttr::type derivedType = rttr::instance(wrappedVar).get_derived_type();

            json j;
            j["$id"] = newId;
            j["$type"] = derivedType.get_name().to_string();
            j["value"] = SerializeClass(wrappedVar);
            return j;
        }
        else if (type.is_arithmetic())
        {
            if (type == rttr::type::get<bool>()) return var.get_value<bool>();
            if (type == rttr::type::get<char>()) return var.get_value<char>();
            if (type == rttr::type::get<int8_t>()) return var.get_value<int8_t>();
            if (type == rttr::type::get<int16_t>()) return var.get_value<int16_t>();
            if (type == rttr::type::get<int32_t>()) return var.get_value<int32_t>();
            if (type == rttr::type::get<int64_t>()) return var.get_value<int64_t>();
            if (type == rttr::type::get<uint8_t>()) return var.get_value<uint8_t>();
            if (type == rttr::type::get<uint16_t>()) return var.get_value<uint16_t>();
            if (type == rttr::type::get<uint32_t>()) return var.get_value<uint32_t>();
            if (type == rttr::type::get<uint64_t>()) return var.get_value<uint64_t>();
            if (type == rttr::type::get<float>()) return var.get_value<float>();
            if (type == rttr::type::get<double>()) return var.get_value<double>();
        }
        else if (type == rttr::type::get<std::string>())
        {
            return var.get_value<std::string>();
        }
        else if (type.is_sequential_container())
        {
            json j = json::array();
            auto view = var.create_sequential_view();
            for (const auto& item : view)
            {
                j.push_back(ToJson(item.extract_wrapped_value()));
            }
            return j;
        }
        else if (type.is_associative_container())
        {
            json j = json::array();
            auto view = var.create_associative_view();
            for (const auto& item : view)
            {
                json itemObj = json::object();
                itemObj["key"] = ToJson(item.first.extract_wrapped_value());
                itemObj["value"] = ToJson(item.second.extract_wrapped_value());
                j.push_back(itemObj);
            }
            
            return j;
        }
        else if (type.is_class())
        {
            return SerializeClass(var);
        }

        HK_CORE_ERROR("Type '{0}' not supported for serialization.", type.get_name().to_string());
        return nullptr;
    }
}
