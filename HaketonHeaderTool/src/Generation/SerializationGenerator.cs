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
            headerContent += "#include \"Haketon/Scene/Entity.h\"\n";
            headerContent += "#include \"Haketon/Scene/Scene.h\"\n";
            headerContent += "#include \"Haketon/Core/Serialization/ISerializationContext.h\"\n\n";
            
            headerContent += "namespace Haketon\n{\n";
            headerContent += "\t// Auto-generated serialization functions\n";
            headerContent += "\tvoid SerializeAllComponents(Entity* entity, ISerializer* serializer);\n";
            headerContent += "\tvoid DeserializeComponent(Entity* entity, IDeserializer* deserializer, const std::string& componentName);\n";
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
            //implContent += "\tusing namespace rapidjson;\n";
            implContent += "\tusing namespace rttr;\n\n";
            
            // Forward declarations
            //implContent += "\t// Forward declarations\n";
            //implContent += "\tvoid Impl_SerializeValue(const variant& Value, PrettyWriter<StringBuffer>& Writer);\n";
            //implContent += "\tbool Impl_DeserializeJsonObject(Value& Object, rttr::variant& OutValue);\n\n";
            
            // Generate serialize function
            implContent += "\tvoid SerializeAllComponents(Entity* entity, ISerializer* serializer)\n\t{\n";
            
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
                implContent += $"\t\t\tserializer->SerializeValue(name.to_string(), comp);\n";
                //implContent += $"\t\t\twriter.String(name.data(), static_cast<SizeType>(name.length()), false);\n";
                //implContent += $"\t\t\tImpl_SerializeValue(comp, writer);\n";
                implContent += "\t\t}\n\n";
            }
            
            implContent += "\t}\n\n";
            
            // Generate deserialize function
            implContent += "\tvoid DeserializeComponent(Entity* entity, IDeserializer* deserializer, const std::string& componentName)\n\t{\n";
            implContent += "\t\tvariant compVar;\n\n";
            
            // Generate if-else chain for component deserialization
            bool isFirst = true;
            foreach (var component in discoveredComponents)
            {
                // Skip the base Component class
                if (component.Name == "Component")
                    continue;
                    
                string conditional = isFirst ? "if" : "else if";
                isFirst = false;
                
                implContent += $"\t\t{conditional}(componentName == \"{component.Name}\")\n\t\t{{\n";
                implContent += $"\t\t\t{component.Name}* comp;\n";
                implContent += $"\t\t\tif(entity->HasComponent<{component.Name}>())\n\t\t\t{{\n";
                implContent += $"\t\t\t\tcomp = &entity->GetComponent<{component.Name}>();\n";
                implContent += "\t\t\t}\n";
                implContent += "\t\t\telse\n\t\t\t{\n";
                implContent += $"\t\t\t\tcomp = &entity->AddComponent<{component.Name}>();\n";
                implContent += "\t\t\t}\n";
                implContent += $"\t\t\tcompVar = comp;\n";
                implContent += $"\t\t\tdeserializer->DeserializeValue(componentName, compVar);\n";
                implContent += $"\t\t\tcomp->OnComponentDeserialized();\n";
                implContent += "\t\t}\n";
            }
            
            implContent += "\t}\n";
            implContent += "}\n";
            
            string implPath = outputDir + "ComponentSerialization.gen.cpp";
            File.WriteAllText(implPath, implContent);
            Console.WriteLine($"Generated serialization implementation: {implPath}");
        }
    }
}