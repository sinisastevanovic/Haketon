using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace HaketonHeaderTool
{
    public static class ComponentRegistryGenerator
    {
        public static void GenerateComponentRegistry(List<ComponentInfo> discoveredComponents, string solutionDir, string outputDir)
        {
            Console.WriteLine($"Generating component registry with {discoveredComponents.Count} discovered components...");
            
            string registryContent = "#include \"Panels/ComponentRegistry.h\"\n";
            registryContent += "#include \"Panels/ComponentSectionHelper.h\"\n\n";
            
            // Read Haketon component registry entries if they exist
            string haketonRegistryPath = solutionDir + "Haketon\\src\\GeneratedFiles\\HaketonComponentRegistryEntries.gen.txt";
            string haketonRegistryEntries = "";
            if (File.Exists(haketonRegistryPath))
            {
                haketonRegistryEntries = File.ReadAllText(haketonRegistryPath);
                Console.WriteLine($"Found Haketon component registry entries at: {haketonRegistryPath}");
                
                // Extract includes from Haketon registry entries
                string[] lines = haketonRegistryEntries.Split('\n');
                foreach (string line in lines)
                {
                    if (line.Trim().StartsWith("#include"))
                    {
                        registryContent += line + "\n";
                    }
                }
            }
            else
            {
                Console.WriteLine($"No Haketon component registry found at: {haketonRegistryPath}");
            }
            
            // Add includes for discovered components (remove duplicates)
            var includedPaths = new HashSet<string>();
            foreach (var component in discoveredComponents)
            {
                if (!includedPaths.Contains(component.IncludePath))
                {
                    registryContent += $"#include \"{component.IncludePath}\"\n";
                    includedPaths.Add(component.IncludePath);
                }
            }
            
            registryContent += "\nnamespace Haketon\n{\n";
            registryContent += "\tvoid InitializeGeneratedComponentRegistry()\n\t{\n";
            registryContent += "\t\tstd::vector<ComponentInfo> registry;\n\n";
            
            // Add Haketon component registry entries
            if (!string.IsNullOrEmpty(haketonRegistryEntries))
            {
                registryContent += "\t\t// Components from Haketon project (auto-generated)\n";
                
                // Extract registry entries (lines between "// Haketon component registry entries:" and "// Template instantiations")
                string[] lines = haketonRegistryEntries.Split('\n');
                bool inRegistrySection = false;
                foreach (string line in lines)
                {
                    if (line.Contains("// Haketon component registry entries:"))
                    {
                        inRegistrySection = true;
                        continue;
                    }
                    if (line.Contains("// Template instantiations"))
                    {
                        inRegistrySection = false;
                        break;
                    }
                    if (inRegistrySection && !string.IsNullOrWhiteSpace(line))
                    {
                        registryContent += line + "\n";
                    }
                }
                registryContent += "\n";
            }
            
            // Generate registry entries for each discovered component from HaketonEditor
            registryContent += "\t\t// Discovered components from HaketonEditor project\n";
            foreach (var component in discoveredComponents)
            {
                registryContent += $"\t\t// Register {component.Name}\n";
                registryContent += "\t\tregistry.push_back({\n";
                registryContent += $"\t\t\trttr::type::get<{component.Name}>(),\n";
                registryContent += $"\t\t\t\"{component.DisplayName}\",\n";
                registryContent += $"\t\t\t{component.IsRemovable.ToString().ToLower()},\n";
                registryContent += $"\t\t\t[](Entity e) {{ return e.HasComponent<{component.Name}>(); }},\n";
                
                if (component.IsRemovable)
                {
                    registryContent += $"\t\t\t[](Entity e) {{ e.AddComponent<{component.Name}>(); }},\n";
                }
                else
                {
                    registryContent += "\t\t\t[](Entity e) { /* Non-addable component */ },\n";
                }
                
                registryContent += $"\t\t\t[](Entity e, bool removable) {{ CreateComponentSection<{component.Name}>(e, removable); }}\n";
                registryContent += "\t\t});\n\n";
            }
            
            registryContent += "\t\tInitializeComponentRegistry(registry);\n";
            registryContent += "\t}\n";
            registryContent += "}\n";
            
            string registryPath = outputDir + "ComponentRegistry.gen.cpp";
            File.WriteAllText(registryPath, registryContent);
            Console.WriteLine($"Generated component registry: {registryPath}");
        }
        
        public static void GenerateHaketonComponentRegistry(List<ComponentInfo> discoveredComponents, string outputDir)
        {
            Console.WriteLine($"Generating Haketon component registry entries with {discoveredComponents.Count} components...");
            
            string registryContent = "// Auto-generated Haketon component registry entries\n";
            registryContent += "// This content should be included in HaketonEditor's component registry\n\n";
            
            // Generate includes
            registryContent += "// Required includes for Haketon components:\n";
            var includedPaths = new HashSet<string>();
            foreach (var component in discoveredComponents)
            {
                if (!includedPaths.Contains(component.IncludePath))
                {
                    // The IncludePath is already relative to the Haketon project src directory
                    string includePath = component.IncludePath.Replace("\\", "/");
                    registryContent += $"#include \"{includePath}\"\n";
                    includedPaths.Add(component.IncludePath);
                }
            }
            
            registryContent += "\n// Haketon component registry entries:\n";
            
            // Generate registry entries (exclude base Component class)
            foreach (var component in discoveredComponents)
            {
                // Skip the base Component class - it's abstract and shouldn't be in the component registry
                if (component.Name == "Component")
                {
                    continue;
                }
                
                registryContent += $"\t\t// Register {component.Name}\n";
                registryContent += "\t\tregistry.push_back({\n";
                registryContent += $"\t\t\trttr::type::get<{component.Name}>(),\n";
                registryContent += $"\t\t\t\"{component.DisplayName}\",\n";
                registryContent += $"\t\t\t{component.IsRemovable.ToString().ToLower()},\n";
                registryContent += $"\t\t\t[](Entity e) {{ return e.HasComponent<{component.Name}>(); }},\n";
                
                if (component.IsRemovable)
                {
                    registryContent += $"\t\t\t[](Entity e) {{ e.AddComponent<{component.Name}>(); }},\n";
                }
                else
                {
                    registryContent += "\t\t\t[](Entity e) { /* Non-addable component */ },\n";
                }
                
                registryContent += $"\t\t\t[](Entity e, bool removable) {{ CreateComponentSection<{component.Name}>(e, removable); }}\n";
                registryContent += "\t\t});\n\n";
            }
            
            // Generate template instantiations (exclude base Component class)
            registryContent += "// Template instantiations for Haketon components:\n";
            foreach (var component in discoveredComponents)
            {
                // Skip the base Component class - it's abstract and shouldn't be in the component registry
                if (component.Name == "Component")
                {
                    continue;
                }
                registryContent += $"\ttemplate void CreateComponentSection<{component.Name}>(Entity entity, bool isRemovable);\n";
            }
            
            string registryPath = outputDir + "HaketonComponentRegistryEntries.gen.txt";
            File.WriteAllText(registryPath, registryContent);
            Console.WriteLine($"Generated Haketon component registry entries: {registryPath}");
        }
    }
}