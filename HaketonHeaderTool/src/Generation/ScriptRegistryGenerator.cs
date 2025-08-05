using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace HaketonHeaderTool
{
    public static class ScriptRegistryGenerator
    {
        public static void GenerateScriptRegistry(List<ScriptInfo> discoveredScripts, string outputDir)
        {
            // Filter only script types (those that derive from ScriptableEntity)
            var scriptComponents = discoveredScripts.Where(c => 
                c.Name != "ScriptableEntity" && 
                c.DerivesFrom("ScriptableEntity")).ToList();

            if (scriptComponents.Count == 0)
            {
                Console.WriteLine("No script components found for script registry generation");
                return;
            }

            Console.WriteLine($"Generating script registry with {scriptComponents.Count} discovered scripts...");
            
            GenerateScriptRegistryHeader(scriptComponents, outputDir);
            GenerateScriptRegistryImplementation(scriptComponents, outputDir);
        }
        
        static void GenerateScriptRegistryHeader(List<ScriptInfo> scriptComponents, string outputDir)
        {
            string headerContent = "#pragma once\n\n";
            headerContent += "#include \"Haketon/Scene/ScriptRegistry.h\"\n\n";
            
            headerContent += "namespace Haketon\n{\n";
            headerContent += "\t// Auto-generated script registration functions\n";
            headerContent += "\tvoid RegisterAllGeneratedScripts();\n";
            headerContent += "}\n";
            
            string headerPath = outputDir + "ScriptRegistration.gen.h";
            File.WriteAllText(headerPath, headerContent);
            Console.WriteLine($"Generated script registry header: {headerPath}");
        }
        
        static void GenerateScriptRegistryImplementation(List<ScriptInfo> scriptComponents, string outputDir)
        {
            string implContent = "#include \"hkpch.h\"\n#include \"ScriptRegistration.gen.h\"\n\n";
            
            // Add includes for discovered scripts (remove duplicates)
            var includedPaths = new HashSet<string>();
            foreach (var script in scriptComponents)
            {
                if (!includedPaths.Contains(script.IncludePath))
                {
                    string includePath = script.IncludePath.Replace("\\", "/");
                    implContent += $"#include \"{includePath}\"\n";
                    includedPaths.Add(script.IncludePath);
                }
            }
            
            implContent += "\nnamespace Haketon\n{\n";
            implContent += "\tvoid RegisterAllGeneratedScripts()\n\t{\n";
            implContent += "\t\tauto& registry = ScriptRegistry::Get();\n\n";
            
            // Generate registration calls for each script
            foreach (var script in scriptComponents)
            {
                implContent += $"\t\t// Register {script.Name}\n";
                implContent += $"\t\tregistry.RegisterScript<{script.Name}>(\"{script.Name}\");\n\n";
            }
            
            implContent += "\t}\n";
            implContent += "}\n";
            
            string implPath = outputDir + "ScriptRegistration.gen.cpp";
            File.WriteAllText(implPath, implContent);
            Console.WriteLine($"Generated script registry implementation: {implPath}");
        }
    }
}