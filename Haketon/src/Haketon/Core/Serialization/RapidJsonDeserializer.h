#pragma once

#include "Haketon/Core/Core.h"
#include "ISerializationContext.h"
#define NOMINMAX
#include <rapidjson/document.h>

namespace rttr
{
    class instance;
    class variant;
    class type;
    class property;
    class method;
}

namespace Haketon
{
    class HK_API RapidJsonDeserializer : public IDeserializer
    {
    public:
        RapidJsonDeserializer();
        ~RapidJsonDeserializer();

        bool Parse(const std::string& jsonString);
        bool ParseFile(const std::string& filePath);

        bool HasObject(const std::string& name) override;
        bool StartObject(const std::string& name = "") override;
        void EndObject() override;
        bool HasArray(const std::string& name) override;
        bool StartArray(const std::string& name = "") override;
        void EndArray() override;
        size_t GetArraySize() override;
        bool HasProperty(const std::string& name) override;

        bool EnterArrayElement(size_t index);
        void ExitArrayElement();

        bool Deserialize(const std::string& name, bool& value) override;
        bool Deserialize(const std::string& name, char& value) override;
        bool Deserialize(const std::string& name, int8_t& value) override;
        bool Deserialize(const std::string& name, int16_t& value) override;
        bool Deserialize(const std::string& name, int32_t& value) override;
        bool Deserialize(const std::string& name, int64_t& value) override;
        bool Deserialize(const std::string& name, uint8_t& value) override;
        bool Deserialize(const std::string& name, uint16_t& value) override;
        bool Deserialize(const std::string& name, uint32_t& value) override;
        bool Deserialize(const std::string& name, uint64_t& value) override;
        bool Deserialize(const std::string& name, float& value) override;
        bool Deserialize(const std::string& name, double& value) override;
        bool Deserialize(const std::string& name, std::string& value) override;

        bool DeserializeValue(const std::string& name, rttr::variant& value) override;

        IReflectable* DeserializeIReflectableObject(const std::string& name, rttr::type baseType) override;
        std::shared_ptr<IReflectable> DeserializeSharedIReflectableObject(const std::string& name, rttr::type baseType) override;
        std::unique_ptr<IReflectable> DeserializeUniqueIReflectableObject(const std::string& name, rttr::type baseType) override;

        bool DeserializeContainer(const std::string& name, rttr::variant_sequential_view& view, rttr::type elementType) override;
        bool DeserializeMap(const std::string& name, rttr::variant_associative_view& view, rttr::type keyType, rttr::type valueType) override;

        bool DeserializeScene(Scene* scene) override;
        bool DeserializeObject(rttr::variant& value) override;

        std::string GetCurrentMemberName() const override { return m_CurrentMemberName; }

    private:
        rapidjson::Document m_Document;
        const rapidjson::Value* m_CurrentValue;
        std::vector<const rapidjson::Value*> m_ValueStack;
        std::string m_CurrentMemberName;

        rttr::variant ExtractBasicTypes(const rapidjson::Value* value);
        rttr::variant ExtractValue(rapidjson::GenericMemberIterator<true, rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>>& itr, const rttr::type& t);

        const rapidjson::Value* GetMember(const std::string& name);

        bool DeserializeProperties(IReflectable* object);
        bool DeserializeProperties(const rttr::instance& instance);
        
        IReflectable* DeserializePolymorphicInternal(const std::string& name, rttr::type baseType);
        void DeserializePolymorphicInternal(const std::string& name, IReflectable* object);
    };

}
