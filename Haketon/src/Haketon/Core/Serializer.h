#pragma once

#include <string>
#include <rttr/type>

namespace Haketon
{
    class Scene;
    class Entity;
    
    class Serializer
    {
    public:

        static std::string SerializeValue(const rttr::variant& Value);
        static std::string SerializeEntity(Entity* Entity);
        static std::string SerializeScene(const Ref<Scene>& Scene, const std::string& FilePath = "");

        static bool DeserializeValue(const std::string& String, rttr::variant& OutValue);
        static Entity DeserializeEntity(const std::string& String, Ref<Scene> Scene);
        static bool DeserializeSceneFromString(const std::string& String, Ref<Scene> Scene);
        static bool DeserializeSceneFromFile(const std::string& FilePath, Ref<Scene> Scene);
    };
}

