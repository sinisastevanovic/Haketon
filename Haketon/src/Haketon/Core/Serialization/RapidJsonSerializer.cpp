#include "hkpch.h"
#include "RapidJsonSerializer.h"

#include <rttr/variant.h>

#include "Haketon/Scene/Entity.h"
#include "Haketon/Scene/Scene.h"

#include "GeneratedFiles/ComponentSerialization.gen.h"

Haketon::RapidJsonSerializer::RapidJsonSerializer()
{
    Reset();
}

Haketon::RapidJsonSerializer::~RapidJsonSerializer()
{
}

void Haketon::RapidJsonSerializer::Reset()
{
    m_Document.SetObject();
    m_CurrentValue = &m_Document;
    m_ValueStack.clear();
}

std::string Haketon::RapidJsonSerializer::GetString() const
{
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    m_Document.Accept(writer);
    return buffer.GetString();
}

void Haketon::RapidJsonSerializer::StartObject(const std::string& name)
{
    rapidjson::Value obj(rapidjson::kObjectType);
    if (!name.empty())
    {
        AddMember(name, obj);
        m_ValueStack.push_back(m_CurrentValue);
        m_CurrentValue = &m_CurrentValue->FindMember(name.c_str())->value;
    }
    else
    {
        // If root object or within an array without name
        // The root object is already set in Reset()
        // For array, push back the new object directly
        if (m_CurrentValue->IsArray())
        {
            m_CurrentValue->PushBack(obj, m_Document.GetAllocator());
            m_ValueStack.push_back(m_CurrentValue);
            m_CurrentValue = &m_CurrentValue->GetArray()[m_CurrentValue->Size() - 1];
        }
        else if (m_ValueStack.empty())
        {
            // This is the root document, already handled by Reset()
        }
        else
        {
            HK_CORE_WARN("Calling StartObject without a name in an unexpected context.");
        }
    }
}

void Haketon::RapidJsonSerializer::EndObject()
{
    if (!m_ValueStack.empty())
    {
        m_CurrentValue = m_ValueStack.back();
        m_ValueStack.pop_back();
    }
}

void Haketon::RapidJsonSerializer::StartArray(const std::string& name)
{
    rapidjson::Value arr(rapidjson::kArrayType);
    if (!name.empty())
    {
        AddMember(name, arr);
        m_ValueStack.push_back(m_CurrentValue);
        m_CurrentValue = &m_CurrentValue->FindMember(name.c_str())->value;
    }
    else
    {
        // This case would be for nested arrays directly added without names,
        // which might occur if you are serializing elements of a container.
        if (m_CurrentValue->IsArray())
        {
            m_CurrentValue->PushBack(arr, m_Document.GetAllocator());
            m_ValueStack.push_back(m_CurrentValue);
            m_CurrentValue = &m_CurrentValue->GetArray()[m_CurrentValue->Size() - 1];
        }
        else
        {
            HK_CORE_WARN("Calling StartArray without a name in an unexpected context.");
        }
    }
}

void Haketon::RapidJsonSerializer::EndArray()
{
    if (!m_ValueStack.empty())
    {
        m_CurrentValue = m_ValueStack.back();
        m_ValueStack.pop_back();
    }
}

void Haketon::RapidJsonSerializer::Serialize(const std::string& name, bool value)
{
    rapidjson::Value v(value);
    AddMember(name, v);
}

void Haketon::RapidJsonSerializer::Serialize(const std::string& name, int value)
{
    rapidjson::Value v(value);
    AddMember(name, v);
}

void Haketon::RapidJsonSerializer::Serialize(const std::string& name, float value)
{
    rapidjson::Value v(value);
    AddMember(name, v);
}

void Haketon::RapidJsonSerializer::Serialize(const std::string& name, double value)
{
    rapidjson::Value v(value);
    AddMember(name, v);
}

void Haketon::RapidJsonSerializer::Serialize(const std::string& name, const std::string& value)
{
    rapidjson::Value v(value.c_str(), m_Document.GetAllocator());
    AddMember(name, v);
}

void Haketon::RapidJsonSerializer::Serialize(const std::string& name, uint64_t value)
{
    rapidjson::Value v(value);
    AddMember(name, v);
}

void Haketon::RapidJsonSerializer::SerializeValue(const std::string& name, const rttr::variant& value)
{
    rttr::type propType = value.get_type();

    if (value.is_type<bool>()) Serialize(name, value.to_bool());
    else if (value.is_type<int>()) Serialize(name, value.to_int());
    else if (value.is_type<float>()) Serialize(name, value.to_float());
    else if (value.is_type<double>()) Serialize(name, value.to_double());
    else if (value.is_type<std::string>()) Serialize(name, value.to_string());
    else if (value.is_type<uint64_t>()) Serialize(name, value.to_uint64());
    else if (propType.is_enumeration())
    {
        bool success = false;
        auto result = value.to_string(&success);
        if (success)
        {
            Serialize(name, result);
        }
        else
        {
            auto intVal = value.to_int();
            Serialize(name, intVal);
        }
    }
    else if (propType.is_sequential_container())
    {
        SerializeContainer(name, value.create_sequential_view());
    }
    else if (propType.is_associative_container())
    {
        SerializeMap(name, value.create_associative_view());
    }
    else if (propType.is_wrapper() && propType.get_wrapped_type().is_derived_from(rttr::type::get<IReflectable>()))
    {
        if (value.get_type().get_name().to_string().find("std::shared_ptr") != std::string::npos)
            SerializeIReflectableObject(name, value.get_value<std::shared_ptr<IReflectable>>());
        else if (value.get_type().get_name().to_string().find("std::unique_ptr") != std::string::npos)
            SerializeIReflectableObject(name, value.get_value<std::unique_ptr<IReflectable>>());
    }
    else if (propType.is_pointer() && propType.get_raw_type().is_derived_from(rttr::type::get<IReflectable>()))
    {
        SerializeIReflectableObject(name, value.get_value<IReflectable*>());
    }
    else if (propType.is_class())
    {
        if (propType.is_wrapper())
            propType = propType.get_wrapped_type();
        
        StartObject(name);
        rttr::method serializeMethod = propType.get_method("Serialize");
        if (serializeMethod.is_valid() && serializeMethod.get_parameter_infos().size() == 1 &&
        serializeMethod.get_parameter_infos().begin()->get_type().is_derived_from(rttr::type::get<ISerializer>())) {
            rttr::instance test(value);
            serializeMethod.invoke(test, ((ISerializer*)this));
        }
        else
        {
            SerializeProperties(value);
        }
        EndObject();
    }
    else
    {
        HK_CORE_ERROR("Serialization Error: Unhandled variant type for property '{0}' of type '{1}'", name, propType.get_name().to_string());
        StartObject(name);
        Serialize("ERROR", "Unhandled Type");
        EndObject();
    }
}

void Haketon::RapidJsonSerializer::SerializeIReflectableObject(const std::string& name, IReflectable* object)
{
    if (!object)
    {
        StartObject(name);
        Serialize("__type__", "null");
        EndObject();
        return;
    }

    rttr::type objectType = rttr::type::get(*object);
    StartObject(name);
    Serialize("__type__", objectType.get_name().to_string());

    rttr::method serializeMethod = objectType.get_method("Serialize");
    if (serializeMethod.is_valid() && serializeMethod.get_parameter_infos().size() == 1 &&
        serializeMethod.get_parameter_infos().begin()->get_type().is_derived_from(rttr::type::get<ISerializer>())) {
        // Call custom Serialize method
        serializeMethod.invoke(*object, ((ISerializer*)this));
        } else {
            // Fallback to generic property serialization
            SerializeProperties(object);
        }
    EndObject();
}

void Haketon::RapidJsonSerializer::SerializeIReflectableObject(const std::string& name, const std::shared_ptr<IReflectable>& object)
{
    SerializeIReflectableObject(name, object.get());
}

void Haketon::RapidJsonSerializer::SerializeIReflectableObject(const std::string& name, const std::unique_ptr<IReflectable>& object)
{
    SerializeIReflectableObject(name, object.get());
}

void Haketon::RapidJsonSerializer::SerializeContainer(const std::string& name, const rttr::variant_sequential_view& view)
{
    StartArray(name);
    for (const auto& item : view)
    {
        SerializeValue("", item);
    }
    EndArray();
}

void Haketon::RapidJsonSerializer::SerializeMap(const std::string& name, const rttr::variant_associative_view& view)
{
    StartObject(name);
    for (const auto& item : view)
    {
        rttr::variant key = item.first;
        rttr::variant value = item.second;

        // TODO:
        // Map keys are typically strings, but can be other types.
        // Convert key to string if possible for JSON object keys.
        // For simplicity here, assuming keys can be converted to string or are primitives.
        if (key.is_type<std::string>() || key.get_type().is_arithmetic())
            SerializeValue(key.to_string(), value);
        else
        {
            break;
        }
    }
    EndObject();
}

void Haketon::RapidJsonSerializer::SerializeScene(const Ref<Scene>& scene)
{
    StartArray("Entities");

    auto view = scene->m_Registry.view<entt::entity>();
    for (auto entityID : view)
    {
        Entity entity = { entityID, scene.get() };
        if(!entity)
            continue;

        SerializeEntity(entity);
    }

    EndArray();
}

void Haketon::RapidJsonSerializer::SerializeEntity(Entity entity)
{
    StartObject();
    SerializeAllComponents(&entity, this);
    EndObject();
}

void Haketon::RapidJsonSerializer::AddMember(const std::string& name, rapidjson::Value& value)
{
    if (m_CurrentValue->IsObject())
    {
        rapidjson::Value n(name.c_str(), m_Document.GetAllocator());
        m_CurrentValue->AddMember(n, value, m_Document.GetAllocator());
    }
    else
    {
        m_CurrentValue->PushBack(value, m_Document.GetAllocator());
    }
}

void Haketon::RapidJsonSerializer::SerializeProperties(IReflectable* object)
{
    if (!object)
        return;

    rttr::type objectType = rttr::type::get(*object);
    for (rttr::property prop : objectType.get_properties())
    {
        if (prop.get_metadata("NoSerialize").is_valid())
            continue;
        
        rttr::variant propValue = prop.get_value(object);
        if (propValue.is_valid())
            SerializeValue(prop.get_name().to_string(), propValue);
        else
            HK_CORE_ERROR("Serialization Error: Could not get value for property '{0}' of type '{1}'", prop.get_name().to_string(), objectType.get_name().to_string());
    }
}

void Haketon::RapidJsonSerializer::SerializeProperties(const rttr::instance& instance)
{
    rttr::type objectType = instance.get_type();
    rttr::instance wrappedInstance = instance.get_wrapped_instance();

    if (wrappedInstance.is_valid())
    {
        objectType = wrappedInstance.get_type();
    }
    
    for (rttr::property prop : objectType.get_properties())
    {
        if (prop.get_metadata("NoSerialize").is_valid())
            continue;
        
        rttr::variant propValue = prop.get_value(wrappedInstance.is_valid() ? wrappedInstance : instance);
        if (propValue.is_valid())
            SerializeValue(prop.get_name().to_string(), propValue);
        else
            HK_CORE_ERROR("Serialization Error: Could not get value for property '{0}' of type '{1}'", prop.get_name().to_string(), objectType.get_name().to_string());
    }
}
