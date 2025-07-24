using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

// TODO: 
/*
 * 3. Performance Optimizations

  // Current: Multiple string operations
  string fileString =
  RemoveComments(File.ReadAllText(headerFileInfo.FullPath));
  
  // Better: Stream processing for large files
  using var reader = new StreamReader(headerFileInfo.FullPath);
  var tokenizer = new CppTokenizer(reader);
  
  4. Missing Features

  - Incremental builds - Only regenerate changed files (check timestamps)
  - Namespace support - Handle nested namespaces properly
  - Template support - Parse template classes/structs
  - Inheritance chains - Better handling of complex inheritance
  - Preprocessor directives - Handle #ifdef, #define properly

  5. Code Generation Issues

  // Current: Hardcoded paths
  string haketonRegistryPath = SolutionDir +
  "Haketon\\src\\GeneratedFiles\\HaketonComponentRegistryEntries.gen.txt";

  // Better: Configurable paths
  var config = HeaderToolConfig.Load();
  string haketonRegistryPath = Path.Combine(config.HaketonOutputDir,
  "HaketonComponentRegistryEntries.gen.txt");

  6. Architecture Improvements

  - Separate parsing from generation - Create CppParser and CodeGenerator
  classes
  - Add validation - Verify generated code compiles
  - Configuration system - External config file for settings
  - Plugin architecture - Allow custom metadata processors

  7. Missing Metadata Support

  Based on your TODO comments, consider adding:
  // Support for additional metadata
  "ReadOnly", "ToolTip", "Category", "AdvancedDisplay", "HideFromParentObject"      

  8. Dependency Management

  - Track file dependencies - Regenerate when included files change
  - Circular dependency detection - Prevent infinite loops
  - Build integration - Better MSBuild/Premake integration

  9. Testing & Validation

  // Add unit tests for parsing logic
  [Test]
  public void Should_ParseComponentMetadata_WhenValidStructFound()
  {
      var result = HeaderTool.ParseStruct("STRUCT(DisplayName=\"Test\") struct      
  TestComponent : Component {};");
      Assert.AreEqual("Test", result.DisplayName);
  }

  10. Performance Monitoring

  // Add timing and progress reporting
  using (var timer = new PerformanceTimer("Parsing"))
  {
      GenerateHeaderForHeader(headerFileInfo);
  }
 */
namespace HaketonHeaderTool
{
    public class Program
    {
        static string SolutionDir = "..\\..\\..\\..\\";
        static string ProjectName = "";
        public static string ProjectSrcDir = "";
        public static string OutputDir = "";

        public static string[] FilesToScan;
        public static List<string> GeneratedFunctions = new List<string>();
        public static List<ComponentInfo> DiscoveredComponents = new List<ComponentInfo>();

        private static List<string> FilesToIgnore = new List<string> { @"Haketon\Core\Core.h", @"hkpch.h", @"Haketon.h" };

        public class ComponentInfo
        {
            public string Name { get; set; }
            public string DisplayName { get; set; }
            public bool IsRemovable { get; set; }
            public string IncludePath { get; set; }
            
            public ComponentInfo(string name, string displayName, bool isRemovable, string includePath)
            {
                Name = name;
                DisplayName = displayName;
                IsRemovable = isRemovable;
                IncludePath = includePath;
            }
        }

        public class HeaderFileInfo
        {
            public string RootSrcDir
            {
                get { return _rootSrcDir; }
                set { _rootSrcDir = value; UpdateCachedString(); }
            }
            
            public string IncludeDir 
            {
                get { return _includeDir; }
                set { _includeDir = value; UpdateCachedString(); }
            }
            public string FileName 
            {
                get { return _fileName; }
                set { _fileName = value; UpdateCachedString(); }
            }

            public string FileNameWithExt
            {
                get { return _fileName + ".h"; }
            }
            
            public string FullPath
            {
                get { return _cachedPath; }
            }

            private string _rootSrcDir = "";
            private string _includeDir = "";
            private string _fileName = "";
            private string _cachedPath = "";

            public HeaderFileInfo() { }

            public HeaderFileInfo(string rootSrcDir, string includeDir, string fileName)
            {
                _rootSrcDir = rootSrcDir;
                _includeDir = includeDir;
                _fileName = fileName;

                UpdateCachedString();
            }

            private void UpdateCachedString()
            {
                _cachedPath = _rootSrcDir + _includeDir + _fileName + ".h";
                
                if(!File.Exists(_cachedPath))
                    Console.WriteLine(string.Format("ERROR: File {0} does not exist!", _cachedPath));
            }
        }

        static void Main(string[] args)
        {
            Logger.Info($"HaketonHeaderTool called with {args.Length} arguments:");
            for (int i = 0; i < args.Length; i++)
            {
                Logger.Info($"  [{i}]: {args[i]}");
            }
            
            try
            {
                if (args.Length < 2)
                {
                    throw new HeaderToolException(
                        "Not enough arguments!\n" +
                        "Usage: HaketonHeaderTool.exe <SolutionDirectory> <ProjectName>\n" +
                        "Example: HaketonHeaderTool.exe C:\\MyProject\\ Haketon");
                }
                
                if (!Directory.Exists(args[0]))
                {
                    throw new HeaderToolException(
                        $"Directory '{args[0]}' does not exist!\n" +
                        "Usage: HaketonHeaderTool.exe <SolutionDirectory> <ProjectName>\n" +
                        "Example: HaketonHeaderTool.exe C:\\MyProject\\ Haketon");
                }
                
                ProcessProject(args[0], args[1]);
                Logger.Info("HeaderTool completed successfully!");
            }
            catch (HeaderToolException ex)
            {
                Logger.Fatal(ex);
                Environment.Exit(1);
            }
            catch (Exception ex)
            {
                Logger.Fatal(ex, "Unexpected error occurred");
                Environment.Exit(1);
            }
            finally
            {
                Logger.Close();
            }
        }
        
        static void ProcessProject(string solutionDir, string projectName)
        {
            SolutionDir = solutionDir;
            ProjectName = projectName;
            ProjectSrcDir = SolutionDir + ProjectName + "\\src\\";
            OutputDir = ProjectSrcDir + "GeneratedFiles\\";

            string dirToSearch = ProjectSrcDir;
            
            if(!Directory.Exists(dirToSearch))
            {
                throw new FileProcessingException($"Project source directory '{dirToSearch}' does not exist!");
            }

            Logger.Info($"Scanning files for project '{ProjectName}'...");

            FilesToScan = Directory.GetFiles(dirToSearch, "*.h", SearchOption.AllDirectories)
                .Where(file => !file.ToLower().EndsWith(".gen.h")).ToArray();
            if (FilesToScan.Length == 0)
            {
                Logger.Warning($"No header files found in project '{ProjectName}'. Skipping code generation.");
                return;
            }
            
            Logger.Info($"Found {FilesToScan.Length} header files to process");
           
            // Delete all previously generated files for current project
            if (Directory.Exists(OutputDir))
            {
                string[] existingFiles = Directory.GetFiles(OutputDir, "*.*", SearchOption.AllDirectories)
                    .Where(file => file.ToLower().EndsWith(".gen.cpp") || file.ToLower().EndsWith(".gen.h")).ToArray();
                foreach (string existingFile in existingFiles)
                {
                    FilesToIgnore.Add(existingFile);
                    File.Delete(existingFile);
                }
            }

            int processedCount = 0;
            int errorCount = 0;
            
            foreach (string file in FilesToScan)
            {
                string currentRelativeDir = file.Replace(dirToSearch, "");
                string fileName = Path.GetFileNameWithoutExtension(currentRelativeDir);
                string includeDir = currentRelativeDir.Replace(fileName + ".h", "");

                if (!FilesToIgnore.Contains(currentRelativeDir))
                {
                    try
                    {
                        HeaderFileInfo headerFileInfo = new HeaderFileInfo(dirToSearch, includeDir, fileName);
                        Logger.Debug($"Processing header: {currentRelativeDir}");
                        
                        if(HeaderParser.GenerateHeaderForHeader(headerFileInfo))
                        {
                            processedCount++;
                            Logger.Debug($"Successfully processed: {currentRelativeDir}");
                        }
                        else
                        {
                            Logger.Debug($"No output generated for: {currentRelativeDir}");
                        }
                    }
                    catch (HeaderToolException ex)
                    {
                        errorCount++;
                        Logger.Error(ex, $"Failed to process {currentRelativeDir}");
                        
                        // Continue processing other files unless it's a fatal error
                        if (ex is FileProcessingException && ex.Message.Contains("does not exist"))
                        {
                            // File not found errors are not fatal, continue
                            continue;
                        }
                    }
                    catch (Exception ex)
                    {
                        errorCount++;
                        Logger.Error(ex, $"Unexpected error processing {currentRelativeDir}");
                        // Continue with other files
                    }
                }
                else
                {
                    Logger.Debug($"Skipping ignored file: {currentRelativeDir}");
                }
            }
            
            Logger.Info($"Processing complete: {processedCount} files processed, {errorCount} errors");
            
            // Generate master registration header
            if (GeneratedFunctions.Count > 0)
            {
                string masterHeaderContent = "#pragma once\n\nnamespace Haketon\n{\n";
                
                // Declare all registration functions
                foreach (string functionName in GeneratedFunctions)
                {
                    masterHeaderContent += "\tvoid " + functionName + "();\n";
                }
                
                // Create master registration function
                masterHeaderContent += "\n\tinline void RegisterAll" + ProjectName + "Types()\n\t{\n";
                foreach (string functionName in GeneratedFunctions)
                {
                    masterHeaderContent += "\t\t" + functionName + "();\n";
                }
                masterHeaderContent += "\t}\n}\n";
                
                string masterHeaderPath = OutputDir + "AutoReflection.gen.h";
                File.WriteAllText(masterHeaderPath, masterHeaderContent);
                Console.WriteLine($"Generated master registration header: {masterHeaderPath}");
            }
                
            // Generate component data for different projects
            if (DiscoveredComponents.Count > 0)
            {
                if (ProjectName == "Haketon")
                {
                    // Generate component registry entries for HaketonEditor to include
                    GenerateHaketonComponentRegistry();
                }
                else if (ProjectName == "HaketonEditor")
                {
                    // Generate complete component registry for HaketonEditor
                    GenerateComponentRegistry();
                }
            }
        }
        
        public static string RemoveComments(string source)
        {
            if (string.IsNullOrEmpty(source))
                return source;
                
            var result = new System.Text.StringBuilder(source.Length);
            int i = 0;
            
            while (i < source.Length)
            {
                char c = source[i];
                
                // Handle string literals (both " and ')
                if (c == '"' || c == '\'')
                {
                    char quote = c;
                    result.Append(c);
                    i++;
                    
                    // Copy everything inside the string literal, handling escape sequences
                    while (i < source.Length)
                    {
                        char stringChar = source[i];
                        result.Append(stringChar);
                        
                        if (stringChar == '\\' && i + 1 < source.Length)
                        {
                            // Skip escaped character
                            i++;
                            result.Append(source[i]);
                        }
                        else if (stringChar == quote)
                        {
                            // End of string literal
                            break;
                        }
                        i++;
                    }
                    i++;
                    continue;
                }
                
                // Handle single-line comments //
                if (c == '/' && i + 1 < source.Length && source[i + 1] == '/')
                {
                    // Skip until end of line
                    i += 2;
                    while (i < source.Length && source[i] != '\r' && source[i] != '\n')
                    {
                        i++;
                    }
                    // Keep the newline character(s)
                    if (i < source.Length && source[i] == '\r')
                    {
                        result.Append('\r');
                        i++;
                    }
                    if (i < source.Length && source[i] == '\n')
                    {
                        result.Append('\n');
                        i++;
                    }
                    continue;
                }
                
                // Handle multi-line comments /* */
                if (c == '/' && i + 1 < source.Length && source[i + 1] == '*')
                {
                    // Skip until */
                    i += 2;
                    while (i + 1 < source.Length)
                    {
                        if (source[i] == '*' && source[i + 1] == '/')
                        {
                            i += 2;
                            break;
                        }
                        // Preserve newlines inside comments to maintain line numbering
                        if (source[i] == '\r' || source[i] == '\n')
                        {
                            result.Append(source[i]);
                        }
                        i++;
                    }
                    continue;
                }
                
                // Regular character
                result.Append(c);
                i++;
            }
            
            return result.ToString();
        }
       
        static void GenerateComponentRegistry()
        {
            Console.WriteLine($"Generating component registry with {DiscoveredComponents.Count} discovered components...");
            
            string registryContent = "#include \"Panels/ComponentRegistry.h\"\n";
            registryContent += "#include \"Panels/ComponentSectionHelper.h\"\n\n";
            
            // Read Haketon component registry entries if they exist
            string haketonRegistryPath = SolutionDir + "Haketon\\src\\GeneratedFiles\\HaketonComponentRegistryEntries.gen.txt";
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
            foreach (var component in DiscoveredComponents)
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
            foreach (var component in DiscoveredComponents)
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
            
            string registryPath = OutputDir + "ComponentRegistry.gen.cpp";
            File.WriteAllText(registryPath, registryContent);
            Console.WriteLine($"Generated component registry: {registryPath}");
            
            // Generate template instantiations file
            // Note: No longer generating explicit template instantiations 
            // Template definition will be in header file instead
        }
        
        static void GenerateHaketonComponentRegistry()
        {
            Console.WriteLine($"Generating Haketon component registry entries with {DiscoveredComponents.Count} components...");
            
            string registryContent = "// Auto-generated Haketon component registry entries\n";
            registryContent += "// This content should be included in HaketonEditor's component registry\n\n";
            
            // Generate includes
            registryContent += "// Required includes for Haketon components:\n";
            var includedPaths = new HashSet<string>();
            foreach (var component in DiscoveredComponents)
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
            foreach (var component in DiscoveredComponents)
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
            foreach (var component in DiscoveredComponents)
            {
                // Skip the base Component class - it's abstract and shouldn't be in the component registry
                if (component.Name == "Component")
                {
                    continue;
                }
                registryContent += $"\ttemplate void CreateComponentSection<{component.Name}>(Entity entity, bool isRemovable);\n";
            }
            
            string registryPath = OutputDir + "HaketonComponentRegistryEntries.gen.txt";
            File.WriteAllText(registryPath, registryContent);
            Console.WriteLine($"Generated Haketon component registry entries: {registryPath}");
        }
    }
}