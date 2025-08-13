#pragma once

#include "Haketon/Core/Core.h"
#include "ISerializationContext.h"
#define NOMINMAX
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <filesystem>

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
    class HK_API RapidJsonSerializer : public ISerializer
    {
    public:
        RapidJsonSerializer();
        ~RapidJsonSerializer();

        // Reset document for new serialization
        void Reset();

        std::string GetString() const;
        bool SaveToFile(const std::filesystem::path& filePath);

        // ISerializer implementation
        void StartObject(const std::string& name = "") override;
        void EndObject() override;
        void StartArray(const std::string& name = "") override;
        void EndArray() override;

        void Serialize(const std::string& name, bool value) override;
        void Serialize(const std::string& name, char value) override;
        void Serialize(const std::string& name, int8_t value) override;
        void Serialize(const std::string& name, int16_t value) override;
        void Serialize(const std::string& name, int32_t value) override;
        void Serialize(const std::string& name, int64_t value) override;
        void Serialize(const std::string& name, uint8_t value) override;
        void Serialize(const std::string& name, uint16_t value) override;
        void Serialize(const std::string& name, uint32_t value) override;
        void Serialize(const std::string& name, uint64_t value) override;
        void Serialize(const std::string& name, float value) override;
        void Serialize(const std::string& name, double value) override;
        void Serialize(const std::string& name, const std::string& value) override;

        void SerializeValue(const std::string& name, const rttr::variant& value) override;

        void SerializeIReflectableObject(const std::string& name, IReflectable* object) override;
        void SerializeIReflectableObject(const std::string& name, const std::shared_ptr<IReflectable>& object) override;
        void SerializeIReflectableObject(const std::string& name, const std::unique_ptr<IReflectable>& object) override;

        void SerializeContainer(const std::string& name, const rttr::variant_sequential_view& view) override;
        void SerializeMap(const std::string& name, const rttr::variant_associative_view& view) override;

        void SerializeScene(Scene* scene) override;
        void SerializeEntity(Entity entity) override;
        void SerializeObject(const rttr::variant& value) override;

        std::string GetCurrentMemberName() const override { return m_CurrentPropName; }

    private:
        rapidjson::Document m_Document;
        rapidjson::Value* m_CurrentValue; // Points to the current object/array being written
        std::vector<rapidjson::Value*> m_ValueStack; // Stack for nested objects/arrays

        std::string m_CurrentPropName;
        
        void AddMember(const std::string& name, rapidjson::Value& value);
        void SerializeProperties(IReflectable* object);
        void SerializeProperties(const rttr::instance& instance);
    };

}

