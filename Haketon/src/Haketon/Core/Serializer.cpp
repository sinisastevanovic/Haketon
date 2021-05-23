#include "hkpch.h"
#include "Serializer.h"

#define RAPIDJSON_HAS_STDSTRING 1

#include <filesystem>
#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>

#include "Haketon/Scene/Entity.h"
#include "Haketon/Scene/Components.h"
#include "Haketon/Scene/Components/CameraComponent.h"
#include "Haketon/Scene/Components/TagComponent.h"

namespace Haketon
{
    using namespace rapidjson;
    using namespace rttr;

    template<typename T>
    bool ConvertNumber(int64_t NumberToConvert, variant& OutConvertedNumber)
    {
        if (NumberToConvert > std::numeric_limits<T>::lowest() && NumberToConvert < std::numeric_limits<T>::max())
        {
            OutConvertedNumber = static_cast<T>(NumberToConvert);
            return true;
        }

        return false;
    }

    template<typename T>
    bool ConvertNumber(uint64_t NumberToConvert, T& OutConvertedNumber)
    {
        if (NumberToConvert > std::numeric_limits<T>::lowest() && NumberToConvert < std::numeric_limits<T>::max())
        {
            OutConvertedNumber = static_cast<T>(NumberToConvert);
            return true;
        }

        return false;
    }
    
    void Impl_SerializeValue(const variant& Value, PrettyWriter<StringBuffer>& Writer)
    {
        auto Type = Value.get_type().is_wrapper() ? Value.get_type().get_wrapped_type() : Value.get_type();

        if(Type.is_arithmetic())
        {
            if(Type == type::get<bool>())
                Writer.Bool(Value.to_bool());
            else if(Type == type::get<char>())
                Writer.String(Value.to_string());
            else if(Type == type::get<int8_t>() || Type == type::get<int16_t>() || Type == type::get<int32_t>())
                Writer.Int(Value.to_int32());
            else if(Type == type::get<int64_t>())
                Writer.Int64(Value.to_int64());
            else if(Type == type::get<uint8_t>() || Type == type::get<uint16_t>() || Type == type::get<uint32_t>())
                Writer.Uint(Value.to_uint32());
            else if(Type == type::get<uint64_t>())
                Writer.Uint64(Value.to_uint64());
            else if (Type == type::get<float>())
                Writer.Double(Value.to_float());
            else if (Type == type::get<double>())
                Writer.Double(Value.to_double());           
        }
        else if(Type.is_enumeration())
        {
            bool Success = false;
            auto Result = Value.to_string(&Success);
            if(Success)
            {
                Writer.String(Result);
            }
            else
            {
                Success = false;
                auto IntVal = Value.to_uint64(&Success);
                if(Success)
                    Writer.Uint64(IntVal);
                else
                    Writer.Null();
            }
        }
        else if(Type == type::get<std::string>() || Type == type::get<const char*>())
        {
            Writer.String(Value.to_string());
        }
        else if(Value.is_sequential_container())
        {
            auto SeqView = Value.create_sequential_view();

            Writer.StartArray();

            for(const auto& Item : SeqView)
            {
                variant WrappedVar = Item.extract_wrapped_value();
                Impl_SerializeValue(WrappedVar, Writer);
            }

            Writer.EndArray();
        }
        else if(Value.is_associative_container())
        {
            static const string_view KeyName("key");
            static const string_view ValueName("value");
            
            auto AssView = Value.create_associative_view();

            Writer.StartArray();

            for(auto& Item : AssView)
            {
                if(AssView.is_key_only_type())
                {
                    Impl_SerializeValue(Item.first, Writer);
                }
                else
                {
                    Writer.StartObject();
                    Writer.String(KeyName.data(), static_cast<SizeType>(KeyName.length()), false);

                    Impl_SerializeValue(Item.first, Writer);

                    Writer.String(ValueName.data(), static_cast<SizeType>(ValueName.length()), false);

                    Impl_SerializeValue(Item.second, Writer);

                    Writer.EndObject();
                }
            }
            
            Writer.EndArray();
        }
        else
        {
            auto ChildProps = Type.get_properties();
            if(ChildProps.size() > 0)
            {
                Writer.StartObject();
                
                for(auto Prop : ChildProps)
                {
                    if(Prop.get_metadata("NoSerialize"))
                        continue;

                    variant PropValue = Prop.get_value(Value);
                    if(!PropValue)
                        continue;

                    const auto Name = Prop.get_name();
                    Writer.String(Name.data(), static_cast<SizeType>(Name.length()), false);

                    Impl_SerializeValue(PropValue, Writer);
                }

                Writer.EndObject();
            }
        }
    }

    template<typename T>
    void Impl_SerializeComponent(Entity* Entity, PrettyWriter<StringBuffer>& Writer)
    {
        if(Entity->HasComponent<T>())
        {
            T& Comp = Entity->GetComponent<T>();
            type Type = type::get(Comp);
            
            const auto Name = Type.get_name();
            Writer.String(Name.data(), static_cast<rapidjson::SizeType>(Name.length()), false);

            Impl_SerializeValue(Comp, Writer);
        }
    }

    void Impl_SerializeEntity(Entity* Entity, PrettyWriter<StringBuffer>& Writer)
    {
        Writer.StartObject();

        Impl_SerializeComponent<TagComponent>(Entity, Writer);
        Impl_SerializeComponent<TransformComponent>(Entity, Writer);
        Impl_SerializeComponent<CameraComponent>(Entity, Writer);
        Impl_SerializeComponent<SpriteRendererComponent>(Entity, Writer);
        
        Writer.EndObject();
    }
    
    std::string Serializer::SerializeValue(const variant& Value)
    {
        StringBuffer Sb;
        PrettyWriter Writer(Sb);

        Impl_SerializeValue(Value, Writer);

        return Sb.GetString();
    }

    std::string Serializer::SerializeEntity(Entity* Entity)
    {
        StringBuffer Sb;
        PrettyWriter Writer(Sb);

        Impl_SerializeEntity(Entity, Writer);
        
        return Sb.GetString();
    }

    std::string Serializer::SerializeScene(const Ref<Scene>& Scene, const std::string& FilePath)
    {
        StringBuffer Sb;
        PrettyWriter Writer(Sb);

        Writer.StartObject();

        Writer.String("Entities");
        Writer.StartArray();

        Scene->m_Registry.each([&](auto entityID)
        {
            Entity entity = { entityID, Scene.get() };
            if(!entity)
                return;

            Impl_SerializeEntity(&entity, Writer);
        });
        
        Writer.EndArray();
        
        Writer.EndObject();
        
        std::string Result = Sb.GetString();
        if(FilePath.length() > 0)
        {
            std::filesystem::path Path = FilePath;
            if(!std::filesystem::exists(Path.parent_path()))
                std::filesystem::create_directory(Path.parent_path());

            std::ofstream Fout(FilePath);
            Fout << Result.c_str();
        }

        return Result;
    }

    /*bool Impl_DeserializeJsonObject(Value& Object, rttr::variant& OutValue);
    variant ExtractValueFromMemberItr(Value::MemberIterator& Itr, const type& T)
    {
        auto& JsonValue = Itr->value;
        variant ExtractedValue;
        Impl_DeserializeJsonObject(JsonValue, ExtractedValue);
        const bool CouldConvert = ExtractedValue.convert(T);
        if(!CouldConvert)
        {
            if(JsonValue.IsObject())
            {
                constructor ctor = T.get_constructor();
                for(auto& Item : T.get_constructors())
                {
                    if(Item.get_instantiated_type() == T)
                        ctor = Item;
                }
                ExtractedValue = ctor.invoke();
                Impl_DeserializeJsonObject()
            }
        }
        else
        {
            ExtractedValue.convert(T);
        }

        return ExtractedValue;
    }*/

    bool Impl_DeserializeJsonObject(Value& Object, rttr::variant& OutValue)
    {
        auto JsonType = Object.GetType();
        const type TrueType = OutValue.get_type().is_wrapper() ? OutValue.get_type().get_wrapped_type() : OutValue.get_type();
        if(JsonType == kFalseType || JsonType == kTrueType)
        {
            if(TrueType == type::get<bool>())
            {
                OutValue = Object.GetBool();
                return true;
            }
        }
        else if (JsonType == kStringType)
        {            
            if(TrueType.is_enumeration())
            {
                auto Enum = TrueType.get_enumeration();
                auto Var = Enum.name_to_value(Object.GetString());
                if(Var)
                {
                    OutValue = Var;
                    return true;
                }

                return false;
            }
            else
            {
                variant Var = static_cast<std::string>(Object.GetString());
                if(Var.convert(TrueType))
                {
                    OutValue = Var;
                    return true;
                }
            }
        }
        else if(JsonType == kNumberType)
        {
            variant temp;
            
            if(TrueType == type::get<uint8_t>() || TrueType == type::get<uint16_t>() || TrueType == type::get<uint32_t>() || TrueType == type::get<uint64_t>()
                || TrueType == type::get<int8_t>() || TrueType == type::get<int16_t>() || TrueType == type::get<int32_t>() || TrueType == type::get<int64_t>()
                || TrueType == type::get<float>() || TrueType == type::get<double>())
            {
                if(Object.IsUint64())
                    OutValue = Object.GetUint64();
                else if(Object.IsInt64())
                    OutValue = Object.GetInt64();
                else if(Object.IsDouble())
                    OutValue = Object.GetDouble();

                OutValue.convert(TrueType);

                return true;
            }
        }
        else if(JsonType == kArrayType)
        {
            if(OutValue.is_sequential_container())
            {
                auto View = OutValue.create_sequential_view();
                if(!View.set_size(Object.Size()) && View.get_size() != Object.Size())
                    return false;

                const type ArrayValueType = View.get_rank_type(1);

                for(SizeType i = 0; i < Object.Size(); ++i)
                {
                    auto& JsonIndexValue = Object[i];
                    variant var = View.get_value(i);
                    if(Impl_DeserializeJsonObject(JsonIndexValue, var))
                    {
                        if(var.convert(ArrayValueType))
                            View.set_value(i, var);
                    }                  
                }

                return true;
            }
            else if(OutValue.is_associative_container())
            {
                auto View = OutValue.create_associative_view();
                auto begin = View.begin();
                variant KeyVar = begin.get_key();
                variant ValVar = begin.get_value();

                View.clear();

                for(SizeType i = 0; i < Object.Size(); ++i)
                {
                    auto& JsonIndexValue = Object[i];
                    if(JsonIndexValue.IsObject()) // a key-value associative view
                    {
                        Value::MemberIterator KeyItr = JsonIndexValue.FindMember("key");
                        Value::MemberIterator ValueItr = JsonIndexValue.FindMember("value");

                        if(KeyItr != JsonIndexValue.MemberEnd() && ValueItr != JsonIndexValue.MemberEnd())
                        {
                            Impl_DeserializeJsonObject(KeyItr->value, KeyVar);
                            Impl_DeserializeJsonObject(ValueItr->value, ValVar);

                            if(KeyVar.convert(View.get_key_type()) && ValVar.convert(View.get_value_type()))
                                View.insert(KeyVar, ValVar);
                        }
                    }
                    else // a key-only associative view
                    {
                        Impl_DeserializeJsonObject(JsonIndexValue, KeyVar);
                        if(KeyVar.convert(View.get_key_type()))
                            View.insert(KeyVar);
                    }
                }

                return true;
            }
        }
        else if(JsonType == kObjectType)
        {
            const auto PropList = TrueType.get_properties();
            for(auto Prop : PropList)
            {
                auto Itr = Object.FindMember(Prop.get_name().data());
                if(Itr == Object.MemberEnd())
                    continue;

                auto PropValue = Prop.get_value(OutValue);
                auto& JsonValue = Itr->value;

                if(Impl_DeserializeJsonObject(JsonValue, PropValue))
                {
                    Prop.set_value(OutValue, PropValue);
                }                
            }

            return true;
        }
        
        return false;
    }
    
    bool Serializer::DeserializeValue(const std::string& String, rttr::variant& OutValue)
    {
        Document document;

        if(document.Parse(String.c_str()).HasParseError())
        {
            ParseErrorCode error = document.GetParseError();         
            return nullptr;
        }
        
        return Impl_DeserializeJsonObject(document, OutValue);
    }

    Entity Impl_DeserializeEntity(Value& Object, Ref<Scene> Scene)
    {
        Entity NewEntity = Scene->CreateEntity();

        for(auto Itr = Object.MemberBegin(); Itr != Object.MemberEnd(); ++Itr)
        {
            variant CompVar;
            if(Itr->name == "TagComponent")
            {
                if(NewEntity.HasComponent<TagComponent>())
                {
                    auto* Comp = &NewEntity.GetComponent<TagComponent>();
                    CompVar = Comp;
                }
                else
                {
                    auto* Comp = &NewEntity.AddComponent<TagComponent>();
                    CompVar = Comp;
                }                
            }
            else if(Itr->name == "TransformComponent")
            {
                if(NewEntity.HasComponent<TransformComponent>())
                {
                    auto* Comp = &NewEntity.GetComponent<TransformComponent>();
                    CompVar = Comp;
                }
                else
                {
                    auto* Comp = &NewEntity.AddComponent<TransformComponent>();
                    CompVar = Comp;
                }        
            }
            else if(Itr->name == "CameraComponent")
            {
                if(NewEntity.HasComponent<CameraComponent>())
                {
                    auto* Comp = &NewEntity.GetComponent<CameraComponent>();
                    CompVar = Comp;
                }
                else
                {
                    auto* Comp = &NewEntity.AddComponent<CameraComponent>();
                    CompVar = Comp;
                }    
            }
            else if(Itr->name == "SpriteRendererComponent")
            {
                if(NewEntity.HasComponent<SpriteRendererComponent>())
                {
                    auto* Comp = &NewEntity.GetComponent<SpriteRendererComponent>();
                    CompVar = Comp;
                }
                else
                {
                    auto* Comp = &NewEntity.AddComponent<SpriteRendererComponent>();
                    CompVar = Comp;
                }
            }

            Impl_DeserializeJsonObject(Itr->value, CompVar);
        }

        return NewEntity;
    }

    Entity Serializer::DeserializeEntity(const std::string& String, Ref<Scene> Scene)
    {
        Document document;

        if(document.Parse(String.c_str()).HasParseError())
        {
            ParseErrorCode error = document.GetParseError();         
            return Entity();
        }

        if(!document.IsObject())
            return Entity();
        
        return Impl_DeserializeEntity(document, Scene);
    }

    bool Serializer::DeserializeSceneFromString(const std::string& String, Ref<Scene> Scene)
    {
        Document document;

        if(document.Parse(String.c_str()).HasParseError())
        {
            ParseErrorCode error = document.GetParseError();         
            return false;
        }

        if(!document.IsObject())
            return false;

        auto EntArray = document.FindMember("Entities");
        if(EntArray != document.MemberEnd())
        {
            auto& EntArrayValue = EntArray->value;
            if(!EntArrayValue.IsArray())
                return false;

            //Scene->DestroyAllEntities();

            for(int32_t i = 0; i < EntArrayValue.Size(); ++i)
            {
                Impl_DeserializeEntity(EntArrayValue[i], Scene);
            }
        }
        
        return true;
    }

    bool Serializer::DeserializeSceneFromFile(const std::string& FilePath, Ref<Scene> Scene)
    {
        std::ifstream Stream(FilePath);
        std::stringstream StrStream;
        StrStream << Stream.rdbuf();

        return DeserializeSceneFromString(StrStream.str(), Scene);
    }
}
