#include "hkpch.h"
#include "RapidJsonDeserializer.h"

#include <filesystem>

#include "Haketon/Core/IReflectable.h"

#include <rttr/registration>
#include <rttr/type>
#include <rttr/property.h>
#include <rttr/method.h>

#include "Haketon/Core/ComponentRegistry.h"
#include "Haketon/Scene/Entity.h"
#include "Haketon/Scene/Scene.h"

Haketon::RapidJsonDeserializer::RapidJsonDeserializer()
{
    m_CurrentValue = nullptr;
} 

Haketon::RapidJsonDeserializer::~RapidJsonDeserializer()
{
}

bool Haketon::RapidJsonDeserializer::Parse(const std::string& jsonString)
{
    m_Document.Parse(jsonString.c_str());
    if (m_Document.HasParseError())
    {
        HK_CORE_ERROR("RapidJSON Parse Error: {0}", (uint32_t)m_Document.GetParseError());
        return false;
    }

    m_CurrentValue = &m_Document;
    m_ValueStack.clear();
    return true;
}

bool Haketon::RapidJsonDeserializer::ParseFile(const std::string& filePath)
{
    if (!std::filesystem::exists(filePath))
        return false;
    
    std::ifstream Stream(filePath);
    std::stringstream StrStream;
    StrStream << Stream.rdbuf();

    return Parse(StrStream.str());
}

bool Haketon::RapidJsonDeserializer::HasObject(const std::string& name)
{
    const rapidjson::Value* member = GetMember(name);
    return member && member->IsObject();
}

bool Haketon::RapidJsonDeserializer::StartObject(const std::string& name)
{
    const rapidjson::Value* targetValue = nullptr;
    if (!name.empty())
    {
        // If a name is provided, expect current value to be an object to find members in
        if (!m_CurrentValue || !m_CurrentValue->IsObject())
        {
            HK_CORE_ERROR("Error: Attempted to StartObject('{0}') but current scope is not an object.", name);
            return false;
        }
        targetValue = GetMember(name);
    }
    else
    {
        // If no name, assume current value IS the object (e.g., root, or after EnterArrayElement)
        targetValue = m_CurrentValue;
    }

    if (targetValue && targetValue->IsObject())
    {
        m_ValueStack.push_back(m_CurrentValue);
        m_CurrentValue = targetValue;
        return true;
    }
    HK_CORE_ERROR("Error: Target value for object '{0}' is not a valid object or not found.", name);
    return false;
}

void Haketon::RapidJsonDeserializer::EndObject()
{
    if (!m_ValueStack.empty())
    {
        m_CurrentValue = m_ValueStack.back();
        m_ValueStack.pop_back();
    }
}

bool Haketon::RapidJsonDeserializer::HasArray(const std::string& name)
{
    const rapidjson::Value* member = GetMember(name);
    return member && member->IsArray();
}

bool Haketon::RapidJsonDeserializer::StartArray(const std::string& name)
{
    const rapidjson::Value* targetValue = nullptr;
    if (!name.empty())
    {
        // If a name is provided, expect current value to be an object to find members in
        if (!m_CurrentValue || !m_CurrentValue->IsObject())
        {
            HK_CORE_ERROR("Error: Attempted to StartArray('{0}') but current scope is not an object.", name);
            return false;
        }
        targetValue = GetMember(name);
    }
    else
    {
        // If no name, assume current value IS the array (e.g., after EnterArrayElement, for a nested array)
        targetValue = m_CurrentValue;
    }

    if (targetValue && targetValue->IsArray())
    {
        m_ValueStack.push_back(m_CurrentValue);
        m_CurrentValue = targetValue;
        return true;
    }
    HK_CORE_ERROR("Error: Target value for array '{0}' is not a valid array or not found.", name);
    return false;
}

void Haketon::RapidJsonDeserializer::EndArray()
{
    if (!m_ValueStack.empty())
    {
        m_CurrentValue = m_ValueStack.back();
        m_ValueStack.pop_back();
    }
}

size_t Haketon::RapidJsonDeserializer::GetArraySize()
{
    if (m_CurrentValue && m_CurrentValue->IsArray())
        return m_CurrentValue->Size();

    return 0;
}

bool Haketon::RapidJsonDeserializer::HasProperty(const std::string& name)
{
    return GetMember(name) != nullptr;
}

bool Haketon::RapidJsonDeserializer::EnterArrayElement(size_t index)
{
    if (!m_CurrentValue || !m_CurrentValue->IsArray())
    {
        HK_CORE_ERROR("Error: Current scope is not an array for EnterArrayElement().");
        return false;
    }
    if (index >= m_CurrentValue->Size())
    {
        HK_CORE_ERROR("Error: Array index {0} out of bounds (size {1}).", index, m_CurrentValue->Size());
        return false;
    }

    m_ValueStack.push_back(m_CurrentValue);
    m_CurrentValue = &(*m_CurrentValue)[index];
    return true;
}

void Haketon::RapidJsonDeserializer::ExitArrayElement()
{
    if (!m_ValueStack.empty())
    {
        m_CurrentValue = m_ValueStack.back();
        m_ValueStack.pop_back();
    }
    else
    {
        // Error: Stack underflow
        m_CurrentValue = nullptr;
        HK_CORE_ERROR("Error: ExitArrayElement called on empty stack.");
    }
}

#define DESERIALIZE_PRIMITIVE(type_name, rapidjson_type, member_type) \
    bool Haketon::RapidJsonDeserializer::Deserialize(const std::string& name, type_name& value) { \
        const rapidjson::Value* member = GetMember(name); \
        if (member && member->Is##rapidjson_type()) { \
            value = member->Get##member_type(); \
            return true; \
        } \
        return false; \
    }

DESERIALIZE_PRIMITIVE(bool, Bool, Bool)
DESERIALIZE_PRIMITIVE(int, Int, Int)
DESERIALIZE_PRIMITIVE(float, Float, Float)
DESERIALIZE_PRIMITIVE(double, Double, Double)
DESERIALIZE_PRIMITIVE(std::string, String, String)
DESERIALIZE_PRIMITIVE(uint64_t, Uint64, Uint64) // For UUID's raw value

#undef DESERIALIZE_PRIMITIVE

bool Haketon::RapidJsonDeserializer::DeserializeValue(const std::string& name, rttr::variant& value)
{
    const rapidjson::Value* member = name.empty() ? m_CurrentValue : GetMember(name);
    if (!member)
        return false;

    rttr::type expectedType = value.get_type();
    bool deserialized = false;

    if (expectedType == rttr::type::get<bool>()) { bool val; if (Deserialize(name, val)) { value = val; deserialized = true; } }
    else if (expectedType == rttr::type::get<int>()) { int val; if (Deserialize(name, val)) { value = val; deserialized = true; } }
    else if (expectedType == rttr::type::get<float>()) { float val; if (Deserialize(name, val)) { value = val; deserialized = true; } }
    else if (expectedType == rttr::type::get<double>()) { double val; if (Deserialize(name, val)) { value = val; deserialized = true; } }
    else if (expectedType == rttr::type::get<std::string>()) { std::string val; if (Deserialize(name, val)) { value = val; deserialized = true; } }
    else if (expectedType == rttr::type::get<uint64_t>()) { uint64_t val; if (Deserialize(name, val)) { value = val; deserialized = true; } }
    else if (expectedType.is_enumeration())
    {
        auto enumeration = expectedType.get_enumeration();
        if (member && member->IsString())
        {
            auto enumVal = enumeration.name_to_value(member->GetString());
            if (enumVal.is_valid())
            {
                value = enumVal;
                deserialized = true;
            }
        }
    }
    else if (expectedType.is_sequential_container())
    {
        if (value.is_valid())
        {
            rttr::variant_sequential_view view = value.create_sequential_view();
            if (DeserializeContainer(name, view, view.get_value_type()))
            {
                deserialized = true;
            }
        }
        else
        {
            HK_CORE_ERROR("Deserialization Error: Variant not a valid sequential container view.");
        }
    }
    else if (expectedType.is_associative_container())
    {
        if (value.is_valid() && value.can_convert<rttr::variant_associative_view>())
        {
            rttr::variant_associative_view view = value.create_associative_view();
            if (DeserializeMap(name, view, view.get_key_type(), view.get_value_type()))
            {
                deserialized = true;
            }
        }
        else
        {
            HK_CORE_ERROR("Deserialization Error: Variant not a valid associative container view.");
        }
    }
    else if (expectedType.is_wrapper() && expectedType.get_wrapped_type().is_derived_from(rttr::type::get<IReflectable>()))
    {
        if (expectedType.get_name().to_string().find("std::shared_ptr") != std::string::npos)
        {
            value = DeserializeSharedIReflectableObject(name, expectedType.get_wrapped_type());
            deserialized = value.is_valid();
        }
        else if (expectedType == rttr::type::get<std::unique_ptr<IReflectable>>())
        {
            // TODO: What to do with this?
            //value = DeserializeUniqueIReflectableObject(name, expectedType.get_wrapped_type());
            //deserialized = value.is_valid();
        }
    }
    else if (expectedType.is_pointer() && expectedType.get_raw_type().is_derived_from(rttr::type::get<IReflectable>()))
    {
        DeserializePolymorphicInternal(name, value.get_value<IReflectable*>());
        //value = DeserializeIReflectableObject(name, expectedType.get_raw_type());
        //deserialized = value.is_valid();
    }
    else if (expectedType.is_class() || expectedType.is_pointer())
    {
        if (expectedType.is_pointer())
            expectedType = expectedType.get_raw_type();

        if (expectedType.is_wrapper())
            expectedType = expectedType.get_wrapped_type();
        
        if (StartObject(name))
        {
            /*if (!value.is_valid() || !value.get_type().is_class() || value.get_type() != expectedType)
            {
                value = expectedType.create();
            }*/
            if (value.is_valid())
            {
                rttr::method deserializeMethod = expectedType.get_method("Deserialize");
                if (deserializeMethod.is_valid() && deserializeMethod.get_parameter_infos().size() == 1 &&
                    deserializeMethod.get_parameter_infos().begin()->get_type().is_derived_from(rttr::type::get<IDeserializer>()))
                {
                    // Found a custom Deserialize(IDeserializer&) method, invoke it
                    deserializeMethod.invoke(value, ((IDeserializer*)this));
                    deserialized = true;
                }
                else
                {
                    deserialized = DeserializeProperties(value);
                }
            }
            EndObject();
        }
        else
        {
            deserialized = false;
        }
    }
    else
    {
        HK_CORE_ERROR("Deserialization Error: Unhandled variant type for property '{0}' of type '{1}'", name, expectedType.get_name().to_string());
        deserialized = false;
    }

    return deserialized;
}

Haketon::IReflectable* Haketon::RapidJsonDeserializer::DeserializeIReflectableObject(const std::string& name, rttr::type baseType)
{
    return DeserializePolymorphicInternal(name, baseType);
}

std::shared_ptr<Haketon::IReflectable> Haketon::RapidJsonDeserializer::DeserializeSharedIReflectableObject(const std::string& name, rttr::type baseType)
{
    IReflectable* obj = DeserializePolymorphicInternal(name, baseType);
    if (!obj)
        return nullptr;
    
    // The object was created by RTTR's create() method, which should have allocated it with new
    // We can safely wrap it in shared_ptr, but we need to be careful about ownership
    return std::shared_ptr<IReflectable>(obj);
}

std::unique_ptr<Haketon::IReflectable> Haketon::RapidJsonDeserializer::DeserializeUniqueIReflectableObject(const std::string& name, rttr::type baseType)
{
    IReflectable* obj = DeserializePolymorphicInternal(name, baseType);
    return std::unique_ptr<IReflectable>(obj);
}

bool Haketon::RapidJsonDeserializer::DeserializeContainer(const std::string& name, rttr::variant_sequential_view& view, rttr::type elementType)
{
    if (!StartArray(name))
        return false;

    view.clear();

    for (size_t i = 0; i < GetArraySize(); ++i)
    {
        m_ValueStack.push_back(m_CurrentValue);
        m_CurrentValue = &m_CurrentValue->GetArray()[i];

        rttr::variant elementValue;

        if (elementType.is_class() && !elementType.is_wrapper())
            elementValue = elementType.create();
        else
            elementValue = rttr::variant(elementType);

        if (DeserializeValue("", elementValue))
            view.insert(view.end(), elementValue);
        else
            HK_CORE_ERROR("Deserialization Error: Failed to deserialize array element of type '{0}'", elementType.get_name().to_string());            

        m_CurrentValue = m_ValueStack.back();
        m_ValueStack.pop_back();
    }

    EndArray();
    return true;
}

bool Haketon::RapidJsonDeserializer::DeserializeMap(const std::string& name, rttr::variant_associative_view& view, rttr::type keyType, rttr::type valueType)
{
    if (!StartObject(name))
        return false;

    view.clear();

    for (auto it = m_CurrentValue->MemberBegin(); it != m_CurrentValue->MemberEnd(); ++it)
    {
        std::string keyStr = it->name.GetString();
        const rapidjson::Value& jsonValue = it->value;

        rttr::variant keyVariant;
        rttr::variant valueVariant;
        bool deserializedKey = false;
        bool deserializedValue = false;

        if (keyType == rttr::type::get<std::string>())
        {
            keyVariant = keyStr;
            deserializedKey = true;
        }
        else if (keyType.is_arithmetic())
        {
            rttr::variant tempKeyVariant = keyType.create();
            if (tempKeyVariant.can_convert<std::string>())
            {
                tempKeyVariant = keyStr;
                const rttr::type keyTypeC = keyType;
                tempKeyVariant.convert(keyTypeC);
                keyVariant = tempKeyVariant;
                deserializedKey = keyVariant.is_valid();
            }
        }
        else
        {
            HK_CORE_ERROR("Deserialization Error: Map key type not supported: {0}", keyType.get_name().to_string());
            continue;
        }

        m_ValueStack.push_back(m_CurrentValue);
        m_CurrentValue = &jsonValue;

        if (valueType.is_class() && !valueType.is_wrapper())
            valueVariant = valueType.create();
        else
            valueVariant = rttr::variant(valueType);

        if (DeserializeValue("", valueVariant))
            deserializedValue = true;
        else
            HK_CORE_ERROR("Deserialization Error: Failed to deserialize map value of type '{0}'", valueType.get_name().to_string());

        if (deserializedKey && deserializedValue && keyVariant.is_valid() && valueVariant.is_valid())
            view.insert(keyVariant, valueVariant);

        m_CurrentValue = m_ValueStack.back();
        m_ValueStack.pop_back();
    }

    EndObject();
    return true;
}

bool Haketon::RapidJsonDeserializer::DeserializeScene(Scene* scene)
{
    if (!HasArray("Entities"))
    {
        HK_CORE_ERROR("Deserialization Error: Scene JSON missing 'Entities' array");
        return false;
    }

    if (!StartArray("Entities"))
    {
        HK_CORE_ERROR("Deserialization Error: Failed to start 'Entities' array");
        return false;
    }

    size_t numEntities = GetArraySize();
    HK_CORE_INFO("Deserializing {0} entities...", numEntities);

    for (size_t i = 0; i < numEntities; ++i)
    {
        if (!EnterArrayElement(i))
        {
            HK_CORE_ERROR("Deserialization Error: Failed to start deserializing entity object {0}", i);
            continue;
        }

        Entity entity = scene->CreateEntity();

        const rapidjson::Value* entityJsonNode = m_CurrentValue;
        if (!entityJsonNode || !entityJsonNode->IsObject())
        {
            HK_CORE_ERROR("Deserialization Error: Entity JSON node is not an object");
            EndObject();
            continue;
        }

        for (auto member_it = entityJsonNode->MemberBegin(); member_it != entityJsonNode->MemberEnd(); ++member_it)
        {
            std::string componentTypeName = member_it->name.GetString();
            const auto info = ComponentRegistry::instance().GetComponentInfo(rttr::type::get_by_name(componentTypeName));
            info->deserialize(&entity, this, componentTypeName);
        }

        ExitArrayElement();
    }

    EndArray();
    return true;
}

const rapidjson::Value* Haketon::RapidJsonDeserializer::GetMember(const std::string& name)
{
    if (!m_CurrentValue || !m_CurrentValue->IsObject())
        return nullptr;

    if (m_CurrentValue->HasMember(name.c_str()))
        return &(*m_CurrentValue)[name.c_str()];

    return nullptr;
}

bool Haketon::RapidJsonDeserializer::DeserializeProperties(IReflectable* object)
{
    if (!object)
    {
        return false;
    }

    bool success = true;
    rttr::type objectType = rttr::type::get(*object);
    for (rttr::property prop : objectType.get_properties())
    {
        if (prop.get_metadata("NoSerialize").is_valid())
            continue;
        
        if (!HasProperty(prop.get_name().to_string()))
            continue;

        rttr::variant propValue = prop.get_value(object);
        if (!propValue.is_valid())
        {
            // If it's a new or default-constructed object, the variant might be empty.
            // For value types, this means propValue will contain a valid instance (default-constructed).
            // For pointer types, it might be null.
            // For properties that are value-type classes (e.g. `Vector3`), `prop.get_value` already returns
            // an `rttr::variant` wrapping the actual object instance, so `propValue.is_valid()` will be true.
            // If it's a pointer and null, it's valid to be null initially.
            // So, `propValue` should generally be valid.
        }

        if (DeserializeValue(prop.get_name().to_string(), propValue))
            prop.set_value(object, propValue);
        else
        {
            HK_CORE_ERROR("Deserialization Error: Failed to deserialize property '{0}' of type '{1}'", prop.get_name().to_string(), objectType.get_name().to_string());
            success = false;
        }
    }
    
    return success;
}

bool Haketon::RapidJsonDeserializer::DeserializeProperties(const rttr::instance& instance)
{
    rttr::type objectType = instance.get_type();
    rttr::instance wrappedInstance = instance.get_wrapped_instance();

    if (wrappedInstance.is_valid())
    {
        objectType = wrappedInstance.get_type();
    }

    bool success = true;
    for (rttr::property prop : objectType.get_properties())
    {
        if (prop.get_metadata("NoSerialize").is_valid())
            continue;
        
        if (!HasProperty(prop.get_name().to_string()))
            continue;

        rttr::variant propValue = prop.get_value(wrappedInstance.is_valid() ? wrappedInstance : instance);
        if (!propValue.is_valid())
        {
            // If it's a new or default-constructed object, the variant might be empty.
            // For value types, this means propValue will contain a valid instance (default-constructed).
            // For pointer types, it might be null.
            // For properties that are value-type classes (e.g. `Vector3`), `prop.get_value` already returns
            // an `rttr::variant` wrapping the actual object instance, so `propValue.is_valid()` will be true.
            // If it's a pointer and null, it's valid to be null initially.
            // So, `propValue` should generally be valid.
        }

        if (DeserializeValue(prop.get_name().to_string(), propValue))
            prop.set_value(wrappedInstance.is_valid() ? wrappedInstance : instance, propValue);
        else
        {
            HK_CORE_ERROR("Deserialization Error: Failed to deserialize property '{0}' of type '{1}'", prop.get_name().to_string(), objectType.get_name().to_string());
            success = false;
        }
    }
    
    return success;
}

Haketon::IReflectable* Haketon::RapidJsonDeserializer::DeserializePolymorphicInternal(const std::string& name, rttr::type baseType)
{
    if (!StartObject(name)) {
        return nullptr; // Object not found or not an object
    }

    std::string typeName;
    if (!HasProperty("__type__") || !Deserialize("__type__", typeName)) {
        HK_CORE_ERROR("Deserialization Error: Object at '{0}' missing '__type__' field.", name);
        EndObject();
        return nullptr;
    }

    if (typeName == "null") {
        EndObject();
        return nullptr;
    }

    rttr::type objectType = rttr::type::get_by_name(typeName);
    if (!objectType.is_valid()) {
        HK_CORE_ERROR("Deserialization Error: Unknown type '{0}' for object at '{1}'.", typeName, name);
        EndObject();
        return nullptr;
    }
    if (!objectType.is_derived_from(baseType) && objectType != baseType) {
        HK_CORE_ERROR("Deserialization Error: Type '{0}' is not derived from or equal to expected base type '{1}' for object at '{2}'.", typeName, baseType.get_name().to_string(), name);
        EndObject();
        return nullptr;
    }

    // Create an instance of the derived type
    rttr::variant objectVariant = objectType.create();
    if (!objectVariant.is_valid()) {
        HK_CORE_ERROR("Deserialization Error: Could not create instance of type '{0}' for object at '{1}'.", typeName, name);
        EndObject();
        return nullptr;
    }

    // Attempt to get a base pointer from the variant
    IReflectable* object = objectVariant.get_value<IReflectable*>();
    if (!object) {
        HK_CORE_ERROR("Deserialization Error: Created object of type '{0}' is not convertible to IReflectable* for object at '{1}'.", typeName, name);
        EndObject();
        return nullptr;
    }

    // Check for a custom virtual Deserialize method
    rttr::method deserializeMethod = objectType.get_method("Deserialize");
    if (deserializeMethod.is_valid() && deserializeMethod.get_parameter_infos().size() == 1 &&
        deserializeMethod.get_parameter_infos().begin()->get_type().is_derived_from(rttr::type::get<IDeserializer>())) {
        // Found a custom Deserialize(IDeserializer&) method, invoke it
        deserializeMethod.invoke(*object, ((IDeserializer*)this));
    } else {
        // No custom method or signature mismatch, use generic property deserialization
        DeserializeProperties(object);
    }

    EndObject(); // End the object scope after deserialization
    return object;
}

void Haketon::RapidJsonDeserializer::DeserializePolymorphicInternal(const std::string& name, IReflectable* object)
{
    if (object == nullptr)
    {
        HK_CORE_ERROR("Trying to deserialize NULL object. Please make sure the object is already constructed!");
        return;
    }
    
    if (!StartObject(name)) {
        return; // Object not found or not an object
    }

    std::string typeName;
    if (!HasProperty("__type__") || !Deserialize("__type__", typeName)) {
        HK_CORE_ERROR("Deserialization Error: Object at '{0}' missing '__type__' field.", name);
        EndObject();
        return;
    }

    if (typeName == "null") {
        EndObject();
        return;
    }

    rttr::type objectType = rttr::type::get_by_name(typeName);
    if (!objectType.is_valid()) {
        HK_CORE_ERROR("Deserialization Error: Unknown type '{0}' for object at '{1}'.", typeName, name);
        EndObject();
        return;
    }

    rttr::instance instance(object);
    if (objectType != instance.get_type().get_raw_type())
    {
        HK_CORE_ERROR("Deserialization Error: Type {0} is not equal to {1}", objectType.get_name().to_string(), instance.get_type().get_raw_type().get_name().to_string());
        EndObject();
        return;
    }
    
    // Check for a custom virtual Deserialize method
    rttr::method deserializeMethod = objectType.get_method("Deserialize");
    if (deserializeMethod.is_valid() && deserializeMethod.get_parameter_infos().size() == 1 &&
        deserializeMethod.get_parameter_infos().begin()->get_type().is_derived_from(rttr::type::get<IDeserializer>())) {
        // Found a custom Deserialize(IDeserializer&) method, invoke it
        deserializeMethod.invoke(object, ((IDeserializer*)this));
    } else {
        // No custom method or signature mismatch, use generic property deserialization
        DeserializeProperties(object);
    }

    EndObject(); // End the object scope after deserialization
}
