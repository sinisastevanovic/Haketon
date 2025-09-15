#include "hkpch.h"
#include "JsonDeserializer.h"
#include <rttr/registration>
#include <rttr/type>
#include <rttr/instance.h>
#include <map>

#include "TypeHandlerRegistry.h"

using json = nlohmann::json;

namespace Haketon
{
    struct JsonDeserializer::Impl
    {
        std::map<uint64_t, rttr::variant> pointerIdMap;

        void Reset()
        {
            pointerIdMap.clear();
        }

        rttr::variant ExtractBasicTypes(const json& j);
        rttr::variant ExtractValue(const json& j, const rttr::type& type);
        void FromJson(const json& j, rttr::variant& var);
    };

    JsonDeserializer::JsonDeserializer() : m_Impl(std::make_unique<JsonDeserializer::Impl>()) {}
    JsonDeserializer::~JsonDeserializer() = default;

    bool JsonDeserializer::DeserializeVar(const std::string& data, rttr::variant& object)
    {
        m_Impl->Reset();
        try
        {
            json jsonData = json::parse(data);
            m_Impl->FromJson(jsonData, object);
            return true;
        }
        catch (const std::exception& e)
        {
            HK_CORE_ERROR("JSON Deserialization failed: {0}", e.what());
            return false;
        }
    }

    rttr::variant JsonDeserializer::Impl::ExtractBasicTypes(const json& j)
    {
        if (j.is_string())
        {
            return j.get<std::string>();
        }
        else if (j.is_boolean())
        {
            return j.get<bool>();
        }
        else if (j.is_number_unsigned())
        {
            return j.get<uint64_t>();
        }
        else if (j.is_number_float())
        {
            return j.get<double>();
        }
        else if (j.is_number_integer())
        {
            return j.get<int64_t>();
        }

        return rttr::variant();
    }

    rttr::variant JsonDeserializer::Impl::ExtractValue(const json& j, const rttr::type& type)
    {
        rttr::variant extractedValue = ExtractBasicTypes(j);
        const bool couldConvert = extractedValue.convert(type);
        if (!couldConvert)
        {
            if (j.is_object())
            {
                rttr::constructor ctor = type.get_constructor();
                for (auto& item : type.get_constructors())
                {
                    if (item.get_instantiated_type() == type)
                        ctor = item;
                }
                extractedValue = ctor.invoke();
                FromJson(j, extractedValue);
            }
        }

        return extractedValue;
    }

    void JsonDeserializer::Impl::FromJson(const json& j, rttr::variant& var)
    {
        if (!var.is_valid())
        {
            HK_CORE_ERROR("Cannot deserialize into an invalid variant.");
            return;
        }

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

        if (auto customDeserializer = TypeHandlerRegistry::GetInstance().FindDeserializer(lookupType))
        {
            customDeserializer(j, var);
            return;
        }

        if (j.is_null())
        {
            var.clear();
            return;
        }

        if (j.is_object())
        {
            if (j.contains("$ref"))
            {
                uint64_t id = j["$ref"];
                if (pointerIdMap.count(id))
                {
                    var = pointerIdMap[id];
                }
                else
                {
                    HK_CORE_ERROR("Unresolved pointer reference with id {0}", id);
                }
                return;
            }

            if (j.contains("$id"))
            {
                uint64_t id = j["$id"];
                std::string typeName = j["$type"];
                rttr::type derivedType = rttr::type::get_by_name(typeName);

                if (!derivedType.is_valid())
                {
                    HK_CORE_ERROR("Could not find type '{0}' for deserialization", typeName);
                    return;
                }

                rttr::variant createdObject;
                if (var.get_type().is_wrapper())
                {
                    bool ctorFound = false;
                    for (auto& ctor : derivedType.get_constructors())
                    {
                        if (ctor.get_instantiated_type().is_wrapper())
                        {
                            createdObject = ctor.invoke();
                            ctorFound = true;
                            break;
                        }
                    }
                    if (!ctorFound)
                    {
                        HK_CORE_ERROR("Could not find a constructor to create a smart pointer for type '{0}'", typeName);
                        return;
                    }
                }
                else
                {
                    createdObject = derivedType.create();
                }

                if (!createdObject.is_valid())
                {
                    HK_CORE_ERROR("Could not create instance of type '{0}'", typeName);
                    return;
                }

                var = createdObject;

                pointerIdMap[id] = var;
                rttr::variant targetToPopulate = var.get_type().is_wrapper() ? var.extract_wrapped_value() : var;
                FromJson(j["value"], targetToPopulate);

               // var = newInstance;
                return;
            }
        }

        if (type.is_arithmetic())
        {
            if (type == rttr::type::get<bool>()) var = j.get<bool>();
            else if (type == rttr::type::get<char>()) var = j.get<char>();
            else if (type == rttr::type::get<int8_t>()) var = j.get<int8_t>();
            else if (type == rttr::type::get<int16_t>()) var = j.get<int16_t>();
            else if (type == rttr::type::get<int32_t>()) var = j.get<int32_t>();
            else if (type == rttr::type::get<int64_t>()) var = j.get<int64_t>();
            else if (type == rttr::type::get<uint8_t>()) var = j.get<uint8_t>();
            else if (type == rttr::type::get<uint16_t>()) var = j.get<uint16_t>();
            else if (type == rttr::type::get<uint32_t>()) var = j.get<uint32_t>();
            else if (type == rttr::type::get<uint64_t>()) var = j.get<uint64_t>();
            else if (type == rttr::type::get<float>()) var = j.get<float>();
            else if (type == rttr::type::get<double>()) var = j.get<double>();
        }
        else if (type == rttr::type::get<std::string>())
        {
            var = j.get<std::string>();
        }
        else if (type.is_sequential_container() && j.is_array())
        {
            auto view = var.create_sequential_view();
            view.set_size(j.size());
            for (size_t i = 0; i < j.size(); ++i)
            {
                rttr::variant valueVar = view.get_value(i).extract_wrapped_value();
                FromJson(j[i], valueVar);
                view.set_value(i, valueVar);
            }
        }
        else if (type.is_associative_container() && j.is_array())
        {
            auto view = var.create_associative_view();
            view.clear();
            for (const auto& entry : j)
            {
                rttr::variant keyVar = ExtractValue(entry["key"], view.get_key_type());
                view.insert(keyVar);
                rttr::variant valueVar = ExtractValue(entry["value"], view.get_value_type());
                rttr::type valueType = valueVar.get_type();

                if (keyVar && valueVar)
                    view.insert(keyVar, valueVar);
            }
        }
        else if (type.is_class() && j.is_object())
        {
            for (auto& prop : lookupType.get_properties())
            {
                std::string propName = prop.get_name().to_string();
                if (j.contains(propName))
                {
                    rttr::variant propVar = prop.get_value(var);
                    FromJson(j[propName], propVar);
                    rttr::type test = propVar.get_type();
                    prop.set_value(var, propVar);
                }
            }
        }
    }
}
