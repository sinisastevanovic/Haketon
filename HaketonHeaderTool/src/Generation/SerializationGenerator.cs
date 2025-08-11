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
            
            headerContent += "namespace Haketon\n{\n";
            headerContent += "\t// Auto-generated serialization functions\n";
            headerContent += $"\tvoid Register{ProjectConfiguration.ProjectName}Components();\n";
            headerContent += $"\tvoid Unregister{ProjectConfiguration.ProjectName}Components();\n";
            headerContent += "}\n";
            string headerPath = outputDir + ProjectConfiguration.ProjectName + "ComponentSerialization.gen.h";
            File.WriteAllText(headerPath, headerContent);
            Console.WriteLine($"Generated serialization header: {headerPath}");
        }
        
        static void GenerateSerializationImplementation(List<ComponentInfo> discoveredComponents, string outputDir)
        {
            string implContent = $"#include \"hkpch.h\"\n#include \"{ProjectConfiguration.ProjectName}ComponentSerialization.gen.h\"\n#include \"Haketon/Core/ComponentRegistry.h\"\n\n";
            
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
            implContent += "\tusing namespace rttr;\n\n";
            
            // Generate serialize function
            implContent += $"\tvoid Register{ProjectConfiguration.ProjectName}Components()\n\t{{\n";
            
            foreach (var component in discoveredComponents)
            {
                // Skip the base Component class
                if (component.Name == "Component")
                    continue;

                implContent += $"\t\tComponentRegistry::instance().RegisterComponent<{component.Name}>(\n";
                implContent += $"\t\t\t/* serialize */ [](Entity* entity, ISerializer* serializer) {{\n";
                implContent += $"\t\t\t\tif(entity->HasComponent<{component.Name}>()) {{\n";
                implContent += $"\t\t\t\t\t{component.Name}& comp = entity->GetComponent<{component.Name}>();\n";
                implContent += $"\t\t\t\t\ttype type = type::get(comp);\n";
                implContent += $"\t\t\t\t\tserializer->SerializeValue(type.get_name().to_string(), comp);\n";
                implContent += $"\t\t\t\t}}\n";
                implContent += $"\t\t\t}},\n";
                
                implContent += $"\t\t\t/* deserialize */ [](Entity* entity, IDeserializer* deserializer, const std::string& componentName) {{\n";
                implContent += $"\t\t\t\tif (componentName == \"{component.Name}\") {{\n";
                implContent += $"\t\t\t\t\t{component.Name}* comp;\n";
                implContent += $"\t\t\t\t\tif (entity->HasComponent<{component.Name}>()) {{\n";
                implContent += $"\t\t\t\t\t\tcomp = &entity->GetComponent<{component.Name}>();\n";
                implContent += $"\t\t\t\t\t}} else {{\n";
                implContent += $"\t\t\t\t\t\tcomp = &entity->AddComponent<{component.Name}>();\n";
                implContent += $"\t\t\t\t\t}}\n";
                implContent += $"\t\t\t\t\trttr::variant compVar = comp;\n";
                implContent += $"\t\t\t\t\tdeserializer->DeserializeValue(componentName, compVar);\n";
                implContent += $"\t\t\t\t\tcomp->OnComponentDeserialized();\n";
                implContent += $"\t\t\t\t}}\n";
                implContent += $"\t\t\t}}\n";
                implContent += $"\t\t);\n";
                implContent += $"\n";
            }
            
            implContent += $"\t}}\n";
            
            implContent += $"\tvoid Unregister{ProjectConfiguration.ProjectName}Components()\n\t{{\n";
            
            foreach (var component in discoveredComponents)
            {
                // Skip the base Component class
                if (component.Name == "Component")
                    continue;

                implContent += $"\t\tComponentRegistry::instance().UnregisterComponent<{component.Name}>();\n";
            }
            
            implContent += $"\t}}\n";
            implContent += $"}}\n";
            
            string implPath = outputDir + ProjectConfiguration.ProjectName + "ComponentSerialization.gen.cpp";
            File.WriteAllText(implPath, implContent);
            Console.WriteLine($"Generated serialization implementation: {implPath}");
        }
    }
}