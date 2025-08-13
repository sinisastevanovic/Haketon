#pragma once

#include "Haketon/Core/Core.h"
#include "Haketon/Core/IReflectable.h"

namespace rttr
{
    class variant;
    class type;
    class variant_sequential_view;
    class variant_associative_view;
}

namespace Haketon
{
    class Entity;
    class Scene;

    class HK_API ISerializer
    {
    public:
        virtual ~ISerializer() = default;

        virtual void StartObject(const std::string& name = "") = 0;
        virtual void EndObject() = 0;
        virtual void StartArray(const std::string& name = "") = 0;
        virtual void EndArray() = 0;

        virtual void Serialize(const std::string& name, bool value) = 0;
        virtual void Serialize(const std::string& name, char value) = 0;
        virtual void Serialize(const std::string& name, int8_t value) = 0;
        virtual void Serialize(const std::string& name, int16_t value) = 0;
        virtual void Serialize(const std::string& name, int32_t value) = 0;
        virtual void Serialize(const std::string& name, int64_t value) = 0;
        virtual void Serialize(const std::string& name, uint8_t value) = 0;
        virtual void Serialize(const std::string& name, uint16_t value) = 0;
        virtual void Serialize(const std::string& name, uint32_t value) = 0;
        virtual void Serialize(const std::string& name, uint64_t value) = 0;
        virtual void Serialize(const std::string& name, float value) = 0;
        virtual void Serialize(const std::string& name, double value) = 0;
        virtual void Serialize(const std::string& name, const std::string& value) = 0;

        virtual void SerializeValue(const std::string& name, const rttr::variant& value) = 0;

        virtual void SerializeIReflectableObject(const std::string& name, IReflectable* object) = 0;
        virtual void SerializeIReflectableObject(const std::string& name, const std::shared_ptr<IReflectable>& object) = 0;
        virtual void SerializeIReflectableObject(const std::string& name, const std::unique_ptr<IReflectable>& object) = 0;

        virtual void SerializeContainer(const std::string& name, const rttr::variant_sequential_view& view) = 0;
        virtual void SerializeMap(const std::string& name, const rttr::variant_associative_view& view) = 0;

        virtual void SerializeScene(Scene* scene) = 0;
        virtual void SerializeEntity(Entity entity) = 0;
        virtual void SerializeObject(const rttr::variant& value) = 0;

        virtual std::string GetCurrentMemberName() const = 0;
    };

    class HK_API IDeserializer
    {
    public:
        virtual ~IDeserializer() = default;

        virtual bool HasObject(const std::string& name) = 0;
        virtual bool StartObject(const std::string& name = "") = 0;
        virtual void EndObject() = 0;
        virtual bool HasArray(const std::string& name) = 0;
        virtual bool StartArray(const std::string& name = "") = 0;
        virtual void EndArray() = 0;
        virtual size_t GetArraySize() = 0;
        virtual bool HasProperty(const std::string& name) = 0;

        virtual bool Deserialize(const std::string& name, bool& value) = 0;
        virtual bool Deserialize(const std::string& name, char& value) = 0;
        virtual bool Deserialize(const std::string& name, int8_t& value) = 0;
        virtual bool Deserialize(const std::string& name, int16_t& value) = 0;
        virtual bool Deserialize(const std::string& name, int32_t& value) = 0;
        virtual bool Deserialize(const std::string& name, int64_t& value) = 0;
        virtual bool Deserialize(const std::string& name, uint8_t& value) = 0;
        virtual bool Deserialize(const std::string& name, uint16_t& value) = 0;
        virtual bool Deserialize(const std::string& name, uint32_t& value) = 0;
        virtual bool Deserialize(const std::string& name, uint64_t& value) = 0;
        virtual bool Deserialize(const std::string& name, float& value) = 0;
        virtual bool Deserialize(const std::string& name, double& value) = 0;
        virtual bool Deserialize(const std::string& name, std::string& value) = 0;

        virtual bool DeserializeValue(const std::string& name, rttr::variant& value) = 0;

        virtual IReflectable* DeserializeIReflectableObject(const std::string& name, rttr::type baseType) = 0;
        virtual std::shared_ptr<IReflectable> DeserializeSharedIReflectableObject(const std::string& name, rttr::type baseType) = 0;
        virtual std::unique_ptr<IReflectable> DeserializeUniqueIReflectableObject(const std::string& name, rttr::type baseType) = 0;

        virtual bool DeserializeContainer(const std::string& name, rttr::variant_sequential_view& view, rttr::type elementType) = 0;
        virtual bool DeserializeMap(const std::string& name, rttr::variant_associative_view& view, rttr::type keyType, rttr::type valueType) = 0;

        virtual bool DeserializeScene(Scene* scene) = 0;

        virtual bool DeserializeObject(rttr::variant& value) = 0;

        virtual rttr::variant CreateDefaultVariant(rttr::type type);

        virtual std::string GetCurrentMemberName() const = 0;
    };
}