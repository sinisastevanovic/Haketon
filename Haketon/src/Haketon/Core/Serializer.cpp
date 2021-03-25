#include "hkpch.h"
#include "Serializer.h"

#define RAPIDJSON_HAS_STDSTRING 1

#include <filesystem>
#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <spdlog/fmt/bundled/format.h>


#include "Haketon/Scene/Entity.h"
#include "Haketon/Scene/Components/CameraComponent.h"
#include "Haketon/Scene/Components/TagComponent.h"

using namespace rttr;
using namespace rapidjson;

namespace
{
    void SerializeObject_Recursively(const instance& Object, PrettyWriter<StringBuffer>& Writer);
    bool SerializePropValue(const variant& PropValue, PrettyWriter<StringBuffer>& Writer);
    
    bool SerializeAtomicType(const type& Type, const variant& PropValue, PrettyWriter<StringBuffer>& Writer)
    {
        if(Type.is_arithmetic())
        {
            if (Type == type::get<bool>())
                Writer.Bool(PropValue.to_bool());
            else if (Type == type::get<char>())
                Writer.Bool(PropValue.to_bool());
            else if (Type == type::get<int8_t>())
                Writer.Int(PropValue.to_int8());
            else if (Type == type::get<int16_t>())
                Writer.Int(PropValue.to_int16());
            else if (Type == type::get<int32_t>())
                Writer.Int(PropValue.to_int32());
            else if (Type == type::get<int64_t>())
                Writer.Int64(PropValue.to_int64());
            else if (Type == type::get<uint8_t>())
                Writer.Uint(PropValue.to_uint8());
            else if (Type == type::get<uint16_t>())
                Writer.Uint(PropValue.to_uint16());
            else if (Type == type::get<uint32_t>())
                Writer.Uint(PropValue.to_uint32());
            else if (Type == type::get<uint64_t>())
                Writer.Uint64(PropValue.to_uint64());
            else if (Type == type::get<float>())
                Writer.Double(PropValue.to_double());
            else if (Type == type::get<double>())
                Writer.Double(PropValue.to_double());

            return true;
        }
        else if(Type.is_enumeration())
        {
            bool Success = false;
            auto result = PropValue.to_string(&Success);
            if(Success)
            {
                Writer.String(result);
            }
            else
            {
                Success = false;
                auto Value = PropValue.to_uint64(&Success);
                if(Success)
                    Writer.Uint64(Value);
                else
                    Writer.Null();
            }

            return true;
        }
        else if(Type == type::get<std::string>())
        {
            Writer.String(PropValue.to_string());
            return true;
        }
        else if(Type == type::get<glm::vec3>())
        {
            glm::vec3 vec = PropValue.get_value<glm::vec3>();
            Writer.String(Haketon::Serializer::SerializeVec3(vec));
            return true;
        }
        else if(Type == type::get<glm::vec4>())
        {
            glm::vec4 vec = PropValue.get_value<glm::vec4>();
            Writer.String(Haketon::Serializer::SerializeVec4(vec));
            return true;
        }

        return false;
    }

    void SerializeArray(const variant_sequential_view& View, PrettyWriter<StringBuffer>& Writer)
    {
        Writer.StartArray();

        for(const auto& Item : View)
        {
            if(Item.is_sequential_container())
            {
                SerializeArray(Item.create_sequential_view(), Writer);
            }
            else
            {
                variant WrappedVar = Item.extract_wrapped_value();
                type ValueType = WrappedVar.get_type();
                if(ValueType.is_arithmetic() || ValueType == type::get<std::string>() || ValueType.is_enumeration())
                {
                    SerializeAtomicType(ValueType, WrappedVar, Writer);
                }
                else
                {
                    SerializeObject_Recursively(WrappedVar, Writer);
                }
            }
        }

        Writer.EndArray();
    }

    void SerializeAssociativeContainer(const variant_associative_view& View, PrettyWriter<StringBuffer>& Writer)
    {
        static const string_view KeyName("key");
        static const string_view ValueName("value");

        Writer.StartArray();

        if(View.is_key_only_type())
        {
            for(auto& Item : View)
                SerializePropValue(Item.first, Writer);
        }
        else
        {
            for(auto& Item : View)
            {
                Writer.StartObject();

                Writer.String(KeyName.data(), static_cast<rapidjson::SizeType>(KeyName.length()), false);

                SerializePropValue(Item.first, Writer);

                Writer.String(ValueName.data(), static_cast<rapidjson::SizeType>(ValueName.length()), false);

                SerializePropValue(Item.second, Writer);

                Writer.EndObject();
            }
        }
    }

    bool SerializePropValue(const variant& PropValue, PrettyWriter<StringBuffer>& Writer)
    {
        auto ValueType = PropValue.get_type();
        auto WrappedType = ValueType.is_wrapper() ? ValueType.get_wrapped_type() : ValueType;
        bool bIsWrapper = WrappedType != ValueType;

        if(SerializeAtomicType(bIsWrapper ? WrappedType : ValueType,
                               bIsWrapper ? PropValue.extract_wrapped_value() : PropValue,
                               Writer))
        {           
        }
        else if(PropValue.is_sequential_container())
        {
            SerializeArray(PropValue.create_sequential_view(), Writer);
        }
        else if(PropValue.is_associative_container())
        {
            SerializeAssociativeContainer(PropValue.create_associative_view(), Writer);
        }
        else
        {
            auto ChildProps = bIsWrapper ? WrappedType.get_properties() : ValueType.get_properties();
            if(!ChildProps.empty())
            {
                SerializeObject_Recursively(PropValue, Writer);
            }
            else
            {
                bool Success = false;
                auto Result = PropValue.to_string(&Success);
                Writer.String(Result);
                
                if(!Success)
                    return false;
            }
        }

        return true;
    }
    
    void SerializeObject_Recursively(const instance& Object, PrettyWriter<StringBuffer>& Writer)
    {
        Writer.StartObject();

        instance RealObject = Object.get_type().get_raw_type().is_wrapper() ? Object.get_wrapped_instance() : Object;

        auto PropertyList = RealObject.get_derived_type().get_properties();
        for (auto Prop : PropertyList)
        {
            if(Prop.get_metadata("NoSerialize"))
                continue;

            variant PropValue = Prop.get_value(RealObject);
            if(!PropValue)
                continue;

            const auto Name = Prop.get_name();
            Writer.String(Name.data(), static_cast<rapidjson::SizeType>(Name.length()), false);
            if(!SerializePropValue(PropValue, Writer))
            {
                HK_CORE_ERROR("Cannot serialize property {0}", Name);
            }
        }
        
        Writer.EndObject();
    }

    void DeserializeObject_Recursively(instance Object, Value& JsonObject);

    variant ExtractBasicTypes(Value& JsonValue)
    {
        switch(JsonValue.GetType())
        {
            case kStringType:
            {
                return std::string(JsonValue.GetString());
            }
            case kNullType:     break;
            case kFalseType:
            case kTrueType:
            {
                return JsonValue.GetBool();
            }
            case kNumberType:
            {
                if (JsonValue.IsInt())
                    return JsonValue.GetInt();
                else if (JsonValue.IsDouble())
                    return JsonValue.GetDouble();
                else if (JsonValue.IsUint())
                    return JsonValue.GetUint();
                else if (JsonValue.IsInt64())
                    return JsonValue.GetInt64();
                else if (JsonValue.IsUint64())
                    return JsonValue.GetUint64();
                break;
            }
            // we handle only the basic types here
            case kObjectType:
            case kArrayType: return variant();
        }

        return variant();
    }

    variant ExtractValue(Value::MemberIterator& Itr, const type& T)
    {
        auto& JsonValue = Itr->value;
        variant ExtractedValue = ExtractBasicTypes(JsonValue);
        const bool CouldConvert = ExtractedValue.convert(T);
        if(!CouldConvert)
        {
            if(JsonValue.IsObject())
            {
                constructor ctor = T.get_constructor();
                for(auto& item : T.get_constructors())
                {
                    if(item.get_instantiated_type() == T)
                        ctor = item;
                }

                ExtractedValue = ctor.invoke();
                DeserializeObject_Recursively(ExtractedValue, JsonValue);
            }
        }

        return ExtractedValue;
    }


    void DeserializeArrayRecursively(variant_sequential_view& View, Value& JsonArrayValue)
    {
        View.set_size(JsonArrayValue.Size());
        for(SizeType i = 0; i < JsonArrayValue.Size(); ++i)
        {
            auto& CurrJsonValue = JsonArrayValue[i];
            if(CurrJsonValue.IsArray())
            {
                auto SubArrayView = View.get_value(i).create_sequential_view();
                DeserializeArrayRecursively(SubArrayView, CurrJsonValue);
            }
            else if(CurrJsonValue.IsObject())
            {
                variant VarTmp = View.get_value(i);
                variant WrappedVar = VarTmp.extract_wrapped_value();
                DeserializeObject_Recursively(WrappedVar, CurrJsonValue);
                View.set_value(i, WrappedVar);
            }
            else
            {
                const type ArrayType = View.get_rank_type(1); // TODO: Maybe we need to change this?
                variant ExtractedValue = ExtractBasicTypes(CurrJsonValue);
                if (ExtractedValue.convert(ArrayType))
                    View.set_value(i, ExtractedValue);
            }
        }
    }


    void DeserializeAssociativeContainerRecusively(variant_associative_view& View, Value& JsonAssoValue)
    {
        for(SizeType i = 0; i < JsonAssoValue.Size(); ++i)
        {
            auto& CurrJsonValue = JsonAssoValue[i];
            if(CurrJsonValue.IsObject()) // a key-value associative view
            {
                Value::MemberIterator KeyItr = CurrJsonValue.FindMember("key");
                Value::MemberIterator ValueItr = CurrJsonValue.FindMember("value");

                if(KeyItr != CurrJsonValue.MemberEnd() && ValueItr != CurrJsonValue.MemberEnd())
                {
                    auto KeyVar = ExtractValue(KeyItr, View.get_key_type());
                    auto ValueVar = ExtractValue(ValueItr, View.get_key_type());
                    if(KeyVar && ValueVar)
                        View.insert(KeyVar, ValueVar);
                }
            }
            else // a key-only associative view
            {
                variant ExtractedValue = ExtractBasicTypes(CurrJsonValue);
                if(ExtractedValue && ExtractedValue.convert(View.get_key_type()))
                    View.insert(ExtractedValue);
            }
        }
    }


    void DeserializeObject_Recursively(instance Object, Value& JsonObject)
    {
        instance RealObject = Object.get_type().get_raw_type().is_wrapper() ? Object.get_wrapped_instance() : Object;
        const auto PropList = RealObject.get_derived_type().get_properties();

        for (auto Prop : PropList)
        {
            Value::MemberIterator Ret = JsonObject.FindMember(Prop.get_name().data());
            if(Ret == JsonObject.MemberEnd())
                continue;

            const type PropType = Prop.get_type();

            auto& JsonValue = Ret->value;
            switch(JsonValue.GetType())
            {
                case kArrayType:
                {
                    variant Var;
                    if(PropType.is_sequential_container())
                    {
                        Var = Prop.get_value(RealObject);
                        auto View = Var.create_sequential_view();
                        DeserializeArrayRecursively(View, JsonValue);
                    }
                    else if(PropType.is_associative_container())
                    {
                        Var = Prop.get_value(RealObject);
                        auto View = Var.create_associative_view();
                        DeserializeAssociativeContainerRecusively(View, JsonValue);
                    }

                    Prop.set_value(RealObject, Var);
                    break;
                }
                case kObjectType:
                {
                    variant Var = Prop.get_value(RealObject);
                    DeserializeObject_Recursively(Var, JsonValue);
                    Prop.set_value(RealObject, Var);
                    break;
                }
                default:
                {
                    variant ExtractedValue = ExtractBasicTypes(JsonValue);
                    if(ExtractedValue.convert(PropType))
                        Prop.set_value(RealObject, ExtractedValue);
                }
            }
        }
    }

    template<typename T>
    void SerializeComponentOnEntity(Haketon::Entity& Entity, PrettyWriter<StringBuffer>& Writer)
    {
        if(Entity.HasComponent<T>())
        {
            //Writer.StartObject();

            
            
            T& Comp = Entity.GetComponent<T>();
            type CompType = rttr::type::get(Comp);                      
            const auto Name = CompType.get_name();
            Writer.String(Name.data(), static_cast<rapidjson::SizeType>(Name.length()), false);
            SerializeObject_Recursively(Comp, Writer);

            //Writer.EndObject();
        }
    }

    double CharToFloat(const char *p) {
        double r = 0.0;
        bool neg = false;
        if (*p == '-') {
            neg = true;
            ++p;
        }
        while (*p >= '0' && *p <= '9') {
            r = (r*10.0) + (*p - '0');
            ++p;
        }
        if (*p == '.') {
            double f = 0.0;
            int n = 0;
            ++p;
            while (*p >= '0' && *p <= '9') {
                f = (f*10.0) + (*p - '0');
                ++p;
                ++n;
            }
            r += f / std::pow(10.0, n);
        }
        if (neg) {
            r = -r;
        }
        return r;
    }

    void SerializeEntity_Impl(Haketon::Entity Entity, PrettyWriter<StringBuffer>& Writer)
    {
        if(!Entity)
            return;

        Writer.StartObject();

        // TODO: I don't want to list them all manually... What about user created components?? This just wouldn't work. THIS SUUUUUCCCKKKSSSSSSS!!!! 

        SerializeComponentOnEntity<Haketon::TagComponent>(Entity, Writer);
        SerializeComponentOnEntity<Haketon::TransformComponent>(Entity, Writer);
        SerializeComponentOnEntity<Haketon::CameraComponent>(Entity, Writer);
        SerializeComponentOnEntity<Haketon::SpriteRendererComponent>(Entity, Writer);

        Writer.EndObject();
    }
}

namespace Haketon
{
    std::string Serializer::SerializeScene(const Ref<Scene>& Scene)
    {
        if(!Scene)
            return std::string();

        StringBuffer Sb;
        PrettyWriter<StringBuffer> Writer(Sb);

        Writer.StartObject();
       
        Writer.String("Entities");
        Writer.StartArray();
        
        Scene->m_Registry.each([&](auto entityID)
        {
            Entity entity = { entityID, Scene.get() };
            if(!entity)
                return;

            SerializeEntity_Impl(entity, Writer);
        });

        Writer.EndArray();
        Writer.EndObject();
            
        return Sb.GetString();
    }

    void Serializer::SerializeScene(const Ref<Scene>& Scene, std::string FilePath)
    {
        std::filesystem::path path = FilePath;
        if(!std::filesystem::exists(path.parent_path()))
            std::filesystem::create_directory(path.parent_path());

        std::ofstream fout(FilePath);
        fout << SerializeScene(Scene).c_str();
    }

    void Serializer::DeserializeScene(const std::string& FilePath, Ref<Scene>& Scene)
    {
        HK_CORE_WARN("Deserialize Scene not implmented yet!");

        std::ifstream stream(FilePath);
        std::stringstream strStream;
        strStream << stream.rdbuf();
        auto Entity = Scene->CreateEntity();
        auto& comp = Entity.AddComponent<CameraComponent>();
        DeserializeObject(strStream.str(), comp);

        
    }

    std::string Serializer::SerializeEntity(Entity Entity)
    {
        if(!Entity)
            return std::string();

        StringBuffer Sb;
        PrettyWriter<StringBuffer> Writer(Sb);

        SerializeEntity_Impl(Entity, Writer);
      
        return Sb.GetString();
    }

    std::string Serializer::SerializeObject(rttr::instance Object)
    {
        if(!Object.is_valid())
            return std::string();

        StringBuffer Sb;
        PrettyWriter<StringBuffer> Writer(Sb);

        SerializeObject_Recursively(Object, Writer);

        return Sb.GetString();
    }

    rttr::instance Serializer::DeserializeObject(const std::string& String, rttr::instance Object)
    {
        Document document;

        if(document.Parse(String.c_str()).HasParseError())
            return rttr::instance();

        DeserializeObject_Recursively(Object, document);

        std::string output = SerializeObject(Object);
        HK_CORE_TRACE(output);
        
        return rttr::instance();
    }

    std::string Serializer::SerializeVec3(glm::vec3 Vec)
    {
        return fmt::format("(X: {0}, Y: {1}, Z: {2})", Vec.x, Vec.y, Vec.z);
    }

    glm::vec3 Serializer::DeserializeVec3(std::string String)
    {
        char xCheck [] = "X:";
        char yCheck [] = "Y:";
        char zCheck [] = "Z:";
        
        char xVal [3], yVal [3], zVal [3];
        float x, y, z;
        
        sscanf(String.c_str(), "(%s %f, %s %f, %s %f)", xVal, &x, yVal, &y, zVal, &z);
        
        if(strcmp(xCheck, xVal) == 0 && strcmp(yCheck, yVal) == 0 && strcmp(zCheck, zVal) == 0 &&
            !isnan(x) && !isnan(y) && !isnan(z))
        {
            return glm::vec3(x, y, z);
        }
        
        return glm::vec3();
    }

    std::string Serializer::SerializeVec4(glm::vec4 Vec)
    {
        return fmt::format("(R: {0}, G: {1}, B: {2}, A: {3})", Vec.r, Vec.g, Vec.b, Vec.a);
    }

    glm::vec4 Serializer::DeserializeVec4(std::string String)
    {
        char rCheck [] = "R:";
        char gCheck [] = "G:";
        char bCheck [] = "B:";
        char aCheck [] = "A:";
        
        char rVal [3], gVal [3], bVal [3], aVal [3];
        float r, g, b, a;
        
        sscanf(String.c_str(), "(%s %f, %s %f, %s %f, %s %f)", rVal, &r, gVal, &g, bVal, &b, aVal, &a);
        
        if(strcmp(rCheck, rVal) == 0 && strcmp(gCheck, gVal) == 0 && strcmp(bCheck, bVal) == 0 && strcmp(aCheck, aVal) == 0&&
            !isnan(r) && !isnan(g) && !isnan(b))
        {
            return glm::vec4(r, g, b, a);
        }
        
        return glm::vec4();
    }
}
