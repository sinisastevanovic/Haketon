using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace HaketonHeaderTool
{
    public static class SerializationGenerator
    {
        public static void GenerateSerializationRegistry(List<ComponentInfo> discoveredComponents, string outputDir)
        {
            Console.WriteLine($"Generating serialization registry with {discoveredComponents.Count} discovered components...");
            
            GenerateSerializationHeader(discoveredComponents, outputDir);
            GenerateSerializationImplementation(discoveredComponents, outputDir);
        }
        
        static void GenerateSerializationHeader(List<ComponentInfo> discoveredComponents, string outputDir)
        {
            string headerContent = "#pragma once\n\n";
            headerContent += "#include <rapidjson/prettywriter.h>\n";
            headerContent += "#include <rapidjson/stringbuffer.h>\n";
            headerContent += "#include <rapidjson/document.h>\n";
            headerContent += "#include \"Haketon/Scene/Entity.h\"\n";
            headerContent += "#include \"Haketon/Scene/Scene.h\"\n\n";
            
            headerContent += "namespace Haketon\n{\n";
            headerContent += "\t// Auto-generated serialization functions\n";
            headerContent += "\tvoid SerializeAllComponents(Entity* entity, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);\n";
            headerContent += "\tHaketon::Entity DeserializeAllComponents(rapidjson::Value& object, Ref<Scene> scene);\n";
            headerContent += "}\n";
            
            string headerPath = outputDir + "ComponentSerialization.gen.h";
            File.WriteAllText(headerPath, headerContent);
            Console.WriteLine($"Generated serialization header: {headerPath}");
        }
        
        static void GenerateSerializationImplementation(List<ComponentInfo> discoveredComponents, string outputDir)
        {
            string implContent = "#include \"hkpch.h\"\n#include \"ComponentSerialization.gen.h\"\n\n";
            
            // Add includes for discovered components (remove duplicates)
            var includedPaths = new HashSet<string>();
            foreach (var component in discoveredComponents)
            {
                if (!includedPaths.Contains(component.IncludePath))
                {
                    string includePath = component.IncludePath.Replace("\\", "/");
                    implContent += $"#include \"{includePath}\"\n";
                    includedPaths.Add(component.IncludePath);
                }
            }
            
            implContent += "\nnamespace Haketon\n{\n";
            implContent += "\tusing namespace rapidjson;\n";
            implContent += "\tusing namespace rttr;\n\n";
            
            // Forward declarations
            implContent += "\t// Forward declarations\n";
            implContent += "\tvoid Impl_SerializeValue(const variant& Value, PrettyWriter<StringBuffer>& Writer);\n";
            implContent += "\tbool Impl_DeserializeJsonObject(Value& Object, rttr::variant& OutValue);\n\n";
            
            // Generate serialize function
            implContent += "\tvoid SerializeAllComponents(Entity* entity, PrettyWriter<StringBuffer>& writer)\n\t{\n";
            implContent += "\t\twriter.StartObject();\n\n";
            
            foreach (var component in discoveredComponents)
            {
                // Skip the base Component class
                if (component.Name == "Component")
                    continue;
                    
                implContent += $"\t\t// Serialize {component.Name}\n";
                implContent += $"\t\tif(entity->HasComponent<{component.Name}>())\n\t\t{{\n";
                implContent += $"\t\t\t{component.Name}& comp = entity->GetComponent<{component.Name}>();\n";
                implContent += $"\t\t\ttype type = type::get(comp);\n";
                implContent += $"\t\t\tconst auto name = type.get_name();\n";
                implContent += $"\t\t\twriter.String(name.data(), static_cast<SizeType>(name.length()), false);\n";
                implContent += $"\t\t\tImpl_SerializeValue(comp, writer);\n";
                implContent += "\t\t}\n\n";
            }
            
            implContent += "\t\twriter.EndObject();\n";
            implContent += "\t}\n\n";
            
            // Generate deserialize function
            implContent += "\tEntity DeserializeAllComponents(Value& object, Ref<Scene> scene)\n\t{\n";
            implContent += "\t\tEntity newEntity = scene->CreateEntity();\n\n";
            implContent += "\t\tfor(auto itr = object.MemberBegin(); itr != object.MemberEnd(); ++itr)\n\t\t{\n";
            implContent += "\t\t\tvariant compVar;\n\n";
            
            // Generate if-else chain for component deserialization
            bool isFirst = true;
            foreach (var component in discoveredComponents)
            {
                // Skip the base Component class
                if (component.Name == "Component")
                    continue;
                    
                string conditional = isFirst ? "if" : "else if";
                isFirst = false;
                
                implContent += $"\t\t\t{conditional}(itr->name == \"{component.Name}\")\n\t\t\t{{\n";
                implContent += $"\t\t\t\t{component.Name}* comp;\n";
                implContent += $"\t\t\t\tif(newEntity.HasComponent<{component.Name}>())\n\t\t\t\t{{\n";
                implContent += $"\t\t\t\t\tcomp = &newEntity.GetComponent<{component.Name}>();\n";
                implContent += "\t\t\t\t}\n";
                implContent += "\t\t\t\telse\n\t\t\t\t{\n";
                implContent += $"\t\t\t\t\tcomp = &newEntity.AddComponent<{component.Name}>();\n";
                implContent += "\t\t\t\t}\n";
                implContent += $"\t\t\t\tcompVar = comp;\n";
                implContent += $"\t\t\t\tImpl_DeserializeJsonObject(itr->value, compVar);\n";
                implContent += $"\t\t\t\tcomp->OnComponentDeserialized();\n";
                implContent += "\t\t\t}\n";
            }
            
            implContent += "\t\t}\n\n";
            implContent += "\t\treturn newEntity;\n";
            implContent += "\t}\n";
            implContent += "}\n";
            
            string implPath = outputDir + "ComponentSerialization.gen.cpp";
            File.WriteAllText(implPath, implContent);
            Console.WriteLine($"Generated serialization implementation: {implPath}");
        }
    }
}