#pragma once

#include "Core/IReflectable.h"
#include <vector>
#include <map>
#include <memory>
#include <string>

#include "Asset/AssetManager.h"
#include "Renderer/Shader.h"

namespace Haketon
{
    CLASS()
    class HK_API SimpleTestObject : public IReflectable
    {
    public:
        SimpleTestObject() {}
        SimpleTestObject(bool null)
        {
            IntValue = 0;
            StringValue = "";
        }
        
        PROPERTY()
        int IntValue = 42;

        PROPERTY()
        std::string StringValue = "Hello";

        RTTR_ENABLE(IReflectable)
    };

    CLASS()
    class HK_API SerializationTestClass : public IReflectable
    {
    public:
        // All primitive types
        PROPERTY()
        bool BoolValue = true;

        PROPERTY()
        char CharValue = 'A';

        PROPERTY()
        int8_t Int8Value = -8;

        PROPERTY()
        int16_t Int16Value = -16;

        PROPERTY()
        int32_t Int32Value = -32;

        PROPERTY()
        int64_t Int64Value = -64;

        PROPERTY()
        uint8_t UInt8Value = 8;

        PROPERTY()
        uint16_t UInt16Value = 16;

        PROPERTY()
        uint32_t UInt32Value = 32;

        PROPERTY()
        uint64_t UInt64Value = 64;

        PROPERTY()
        float FloatValue = 3.14f;

        PROPERTY()
        double DoubleValue = 2.71828;

        PROPERTY()
        std::string StringValue = "Test String";

        // Object types
        PROPERTY()
        SimpleTestObject NestedObject;

        // TODO: This does not work!
        PROPERTY()
        std::shared_ptr<SimpleTestObject> SharedPtrObject = std::make_shared<SimpleTestObject>();

        // TODO: This does not work!
        PROPERTY()
        Ref<Shader> m_Shader;

        PROPERTY()
        AssetHandle Handle;

        // Container types
        PROPERTY()
        std::vector<int> IntVector = {1, 2, 3, 4, 5};

        PROPERTY()
        std::vector<std::string> StringVector = {"one", "two", "three"};

        PROPERTY()
        std::vector<SimpleTestObject> ObjectVector;

        PROPERTY()
        std::map<std::string, int> StringIntMap = {{"first", 1}, {"second", 2}, {"third", 3}};

        PROPERTY()
        std::map<int, std::string> IntStringMap = {{1, "one"}, {2, "two"}, {3, "three"}};

        // TODO: This does not work!
        PROPERTY()
        std::map<std::string, SimpleTestObject> ObjectMap;

        SerializationTestClass()
        {
            // Initialize object containers
            ObjectVector.resize(2);
            ObjectVector[0].IntValue = 100;
            ObjectVector[0].StringValue = "First";
            ObjectVector[1].IntValue = 200;
            ObjectVector[1].StringValue = "Second";

            SimpleTestObject obj1, obj2;
            obj1.IntValue = 300;
            obj1.StringValue = "MapObj1";
            obj2.IntValue = 400;
            obj2.StringValue = "MapObj2";
            ObjectMap["object1"] = obj1;
            ObjectMap["object2"] = obj2;

            m_Shader = AssetManager::GetAsset<Shader>(AssetHandle(18077323912996576500));
        }

        SerializationTestClass(bool null)
        {
            BoolValue = false;
            CharValue = '0';
            Int8Value = 0;
            Int16Value = 0;
            Int32Value = 0;
            Int64Value = 0;
            UInt8Value = 0;
            UInt16Value = 0;
            UInt32Value = 0;
            UInt64Value = 0;
            FloatValue = 0;
            DoubleValue = 0;
            StringValue = "";
            NestedObject = SimpleTestObject(true);
            SharedPtrObject = nullptr;
            m_Shader = nullptr;
            Handle = AssetHandle::Null();
            IntVector = {};
            StringVector = {};
            StringIntMap = {};
            IntStringMap = {};
        }
    };
}
