#pragma once

#include <string>
#include <rttr/type>
#include <glm/glm.hpp>

namespace Haketon
{
    class Scene;
    class Entity;

    class Serializer
    {
    public:

        static std::string SerializeScene(const Ref<Scene>& Scene);
        static void SerializeScene(const Ref<Scene>& Scene, std::string FilePath);
        static void DeserializeScene(const std::string& FilePath, Ref<Scene>& Scene);
        
        static std::string SerializeEntity(Entity Entity);
        static std::string SerializeObject(rttr::instance Object);
        static rttr::instance DeserializeObject(const std::string& String, rttr::instance Object);

        static std::string SerializeVec3(glm::vec3 Vec);
        static glm::vec3 DeserializeVec3(std::string String);

        static std::string SerializeVec4(glm::vec4 Vec);
        static glm::vec4 DeserializeVec4(std::string String);
    
    private:

        //static void SerializeObject_Recursively(const instance& Object, PrettyWriter<StringBuffer>& Writer);
        //static bool SerializePropValue(const variant& PropValue, PrettyWriter<StringBuffer>& Writer);
        //static bool SerializeAtomicType(const type& Type, const variant& PropValue, PrettyWriter<StringBuffer>& Writer);
        //static void SerializeArray(const variant_sequential_view& View, PrettyWriter<StringBuffer>& Writer);
        //static void SerializeAssociativeContainer(const variant_associative_view& View, PrettyWriter<StringBuffer>& Writer);



        Serializer() {}
    };

    
}

