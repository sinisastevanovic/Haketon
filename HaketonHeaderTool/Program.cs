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
        private const string StructToken = "STRUCT";
        private const string ClassToken = "CLASS";
        private const string PropertyToken = "PROPERTY";
        private const string EnumToken = "ENUM";
        private const string FunctionToken = "FUNCTION";

        static string SolutionDir = "..\\..\\..\\..\\";
        static string ProjectName = "";
        public static string ProjectSrcDir = "";
        public static string OutputDir = "";
        static string EditorOutputDir = "";

        public static string[] FilesToScan;
        public static List<string> GeneratedFunctions = new List<string>();
        public static List<ComponentInfo> DiscoveredComponents = new List<ComponentInfo>();

        private static List<string> FilesToIgnore = new List<string> { @"Haketon\Core\Core.h", @"hkpch.h", @"Haketon.h" };

        private static readonly char[] NewLineChars = new[] {'\r', '\n'};

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
            EditorOutputDir = SolutionDir + "HaketonEditor\\src\\GeneratedFiles\\";

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

        static bool GenerateHeaderForHeader(HeaderFileInfo headerFileInfo)
        {
            if (!File.Exists(headerFileInfo.FullPath))
                return false;

            string fileString = RemoveComments(File.ReadAllText(headerFileInfo.FullPath));

            if (fileString.Length <= 0)
                return false;


            string registrationString = "";
            // Collect enums
            int enumTokenPos = fileString.IndexOf(EnumToken, StringComparison.Ordinal);
            while (enumTokenPos != -1)
            {
                int endOfLinePos = fileString.IndexOfAny(NewLineChars, enumTokenPos);
                if (endOfLinePos == -1)
                {
                    Console.WriteLine("ENUM declared at the end of the file. Ignoring...");
                    endOfLinePos = -1;
                    continue;
                }
                
                int startOfEnumDecl = FindFirstNotOf(fileString, " \n\r\t", endOfLinePos);
                int endOfEnumDecl = fileString.IndexOfAny(NewLineChars, startOfEnumDecl);
                if (endOfEnumDecl == -1)
                    return false;
                
                string enumDecl = fileString.Substring(startOfEnumDecl, endOfEnumDecl - startOfEnumDecl);
                string enumName;

                if (enumDecl.StartsWith("enum class "))
                {
                    enumName = enumDecl.Substring(11);
                }
                else if (enumDecl.StartsWith("enum "))
                    enumName = enumDecl.Substring(5);
                else
                {
                    Console.WriteLine("ENUM specifier used on non enum type!");
                    return false;
                }
                
                // Find end of enum
                int openingBracketPos = fileString.IndexOf('{', endOfEnumDecl);
                int endOfEnumPos = FindClosingBracket(fileString, openingBracketPos);
                if (endOfEnumPos == -1 || openingBracketPos == -1)
                {
                    Console.WriteLine(string.Format("ERROR: No closing bracket found for enum {0}", enumName));
                    return false;
                }
                
                registrationString += "\n\n\t\tregistration::enumeration<" + enumName + ">(\"" + enumName + "\")\n\t\t\t(";
                
                string enumSource = RemoveWhitespace(fileString.Substring(openingBracketPos + 1, endOfEnumPos - (openingBracketPos + 1)));
                string[] enumValues = enumSource.Split(',');
                for (int i = 0; i < enumValues.Length; i++)
                {
                    string enumValueName = enumValues[i];
                    int equalPos = enumValueName.IndexOf('=');
                    if (equalPos != -1)
                        enumValueName = enumValueName.Substring(0, equalPos);

                    if (i > 0)
                        registrationString += ",";
                    registrationString += "\n\t\t\t\tvalue(\"" + enumValueName + "\",\t" + enumName + "::" + enumValueName + ")";
                }

                registrationString += "\n\t\t\t);";
                
                enumTokenPos = fileString.IndexOf(EnumToken, endOfEnumDecl, StringComparison.Ordinal);
            }
            
            // Collect structs
            int structTokenPos = fileString.IndexOf(StructToken, StringComparison.Ordinal);
            while (structTokenPos != -1)
            {
                int endOfLinePos = fileString.IndexOfAny(NewLineChars, structTokenPos);
                if (endOfLinePos == -1)
                {
                    Console.WriteLine("STRUCT declared at the end of the file. Ignoring...");
                    structTokenPos = -1;
                    continue;
                }

                // Parse STRUCT metadata first
                int beginOfMetadata = structTokenPos + StructToken.Length + 1;
                int endOfMetadata = fileString.IndexOf(')', structTokenPos);
                string metadataString = "";
                if (endOfMetadata != -1 && beginOfMetadata < endOfMetadata)
                {
                    metadataString = fileString.Substring(beginOfMetadata, endOfMetadata - beginOfMetadata);
                }

                int startOfStructDecl = FindFirstNotOf(fileString, " \n\r\t", endOfLinePos);
                int endOfStructDecl = fileString.IndexOfAny(NewLineChars, startOfStructDecl);

                string structDecl = fileString.Substring(startOfStructDecl, endOfStructDecl - startOfStructDecl);
                string structName;


                if (structDecl.StartsWith("struct "))
                {
                    int posOfSpace = structDecl.IndexOf(' ', 7);
                    if (posOfSpace != -1)
                        structName = structDecl.Substring(7, structDecl.IndexOf(' ', 7) - 7);
                    else
                        structName = structDecl.Substring(7);
                }
                else if (structDecl.StartsWith("class "))
                {
                    int posOfSpace = structDecl.IndexOf(' ', 6);
                    if (posOfSpace != -1)
                        structName = structDecl.Substring(6, structDecl.IndexOf(' ', 6) - 6);
                    else
                        structName = structDecl.Substring(6);
                }
                else
                {
                    Console.WriteLine("STRUCT specifier used on non struct type!");
                    return false;
                }

                
                // Find end of struct
                int endOfStructPos = FindClosingBracket(fileString, fileString.IndexOf('{', endOfStructDecl));
                if (endOfStructPos == -1)
                {
                    Console.WriteLine(string.Format("ERROR: No closing bracket found for struct {0}", structName));
                    return false;
                }

                // Collect Properties
                string structSource = fileString.Substring(endOfStructDecl, endOfStructPos - endOfStructDecl);
                
                // Check if this struct inherits from Component
                bool isComponent = false;
                if (structSource.Contains("RTTR_ENABLE(Component)") || structDecl.Contains(": Component") || structDecl.Contains(":Component"))
                {
                    isComponent = true;
                    
                    // Parse component metadata
                    string displayName = ExtractDisplayName(metadataString, structName);
                    bool isRemovable = !metadataString.Contains("NonRemovable");
                    
                    string includePath = headerFileInfo.IncludeDir + headerFileInfo.FileNameWithExt;
                    ComponentInfo componentInfo = new ComponentInfo(structName, displayName, isRemovable, includePath);
                    DiscoveredComponents.Add(componentInfo);
                    
                    Console.WriteLine($"Found component: {structName} (Display: '{displayName}', Removable: {isRemovable})");
                }
                
                registrationString += "\n\n\t\tregistration::class_<" + structName + ">(\"" + structName + "\")\n\t\t\t.constructor()" + ParsePropertiesInClass(structSource, structName);

                // Find next struct
                structTokenPos = fileString.IndexOf(StructToken, endOfStructDecl, StringComparison.Ordinal);
            }

            if (registrationString.Length > 0)
            {
                string includeString = "#include \"" + headerFileInfo.IncludeDir + headerFileInfo.FileNameWithExt + "\"\r\n" + ScanFileForAdditionalIncludes(fileString, headerFileInfo.FileName); 
                string genFileString = "#include \"hkpch.h\"\n" + includeString + "\n\n#include <rttr/registration>\n\n\nnamespace Haketon\n{\n\tvoid Register" + headerFileInfo.FileName + "Types()\n\t{\n\t\tusing namespace rttr;\n\t\tstatic bool registered = false;\n\t\tif (registered) return;\n\t\tregistered = true;\n\t\t\n";
                
                // Convert RTTR_REGISTRATION content to explicit registration calls
                string[] regLines = registrationString.Split('\n');
                foreach (string line in regLines)
                {
                    if (!string.IsNullOrWhiteSpace(line))
                    {
                        genFileString += "\t\t" + line.Trim() + "\n";
                    }
                }
                
                genFileString += "\t}\n}";

                Directory.CreateDirectory(OutputDir);
                File.WriteAllText(OutputDir + headerFileInfo.FileName + ".gen.cpp", genFileString);
                
                // Track this function for the master header
                GeneratedFunctions.Add("Register" + headerFileInfo.FileName + "Types");
            }
            
            return true;
        }

        static string ScanFileForAdditionalIncludes(string source, string headerFileName)
        {
            string includeString = "";
            
            const string classString = "class ";
            const string structString = "struct ";
            
            // Are there class forward declarations?
            List<string> forwardDeclaredClasses = new List<string>();
            
            string[] typesToSearchFor = { "class ", "struct " };
            int curentTypePos;
            int curentTypeIndex;
            int currentOffset = 0;
            
            while (FirstIndexOfAnyString(source, typesToSearchFor, out curentTypePos, out curentTypeIndex, currentOffset))
            {
                int endOfDeclPos = source.IndexOf(';', curentTypePos);
                int endOfLinePos = source.IndexOfAny(NewLineChars, curentTypePos);
                if (endOfDeclPos > -1 && ((endOfDeclPos <= endOfLinePos) || endOfLinePos == -1))
                {
                    string typeDeclaration = source.Substring(curentTypePos + typesToSearchFor[curentTypeIndex].Length, endOfDeclPos - (curentTypePos + typesToSearchFor[curentTypeIndex].Length));
                    int spacePos = typeDeclaration.IndexOf(' ');
                    if (spacePos == -1) // If there is a space, something is wrong...
                    {
                        forwardDeclaredClasses.Add(typeDeclaration);
                    }
                }

                currentOffset = endOfDeclPos;
            }

            
            currentOffset = 0;
            foreach (string forwardDeclaredClass in forwardDeclaredClasses)
            {
                bool bFoundClass = false;
                // First see if there is a file called like the class... Maybe we can skip deep searching.
                foreach (string file in FilesToScan)
                {
                    if (file.Contains(forwardDeclaredClass + ".h"))
                    {
                        string fileSource = File.ReadAllText(file);
                        while (FirstIndexOfAnyString(fileSource, typesToSearchFor, out curentTypePos, out curentTypeIndex, currentOffset))
                        {
                            int endOfStructPos = FindClosingBracket(fileSource, fileSource.IndexOf('{', curentTypePos));
                            int endOfDecl = fileSource.IndexOf(' ', curentTypePos + typesToSearchFor[curentTypeIndex].Length);
                            if(endOfDecl == -1)
                                endOfDecl = fileSource.IndexOfAny(NewLineChars, curentTypePos);
                            
                            if (endOfStructPos != -1 && endOfDecl != -1)
                            {
                                string typeDeclaration = RemoveWhitespace(fileSource.Substring(curentTypePos + typesToSearchFor[curentTypeIndex].Length, endOfDecl - (curentTypePos + typesToSearchFor[curentTypeIndex].Length)));
                                if (typeDeclaration == forwardDeclaredClass)
                                {
                                    bFoundClass = true;
                                    includeString += "#include \"" + file.Replace(ProjectSrcDir, "") + "\"\r\n";
                                }
                                    
                            }

                            currentOffset = endOfDecl;
                        }
                    }
                }
                
                if(!bFoundClass)
                    Console.WriteLine("ERROR: Class {0} not found in files... Maybe it is in file not called after the class? IMPLEMENT THIS!", forwardDeclaredClass);
            }

            return includeString;
            /*foreach (string file in FilesToScan)
            {
                string currentRelativeDir = file.Replace(dirToSearch, "");
                string fileName = Path.GetFileNameWithoutExtension(currentRelativeDir);
                string includeDir = currentRelativeDir.Replace(fileName + ".h", "");
                HeaderFileInfo headerFileInfo = new HeaderFileInfo(dirToSearch, includeDir, fileName);
                Console.WriteLine(string.Format("Generating header for {0}", currentRelativeDir));
                if(GenerateHeaderForHeader(headerFileInfo))
                    Console.WriteLine("Success!");
                else
                    Console.WriteLine("Failed generating header for {0}!", currentRelativeDir);
            }*/
        }

        static bool FirstIndexOfAnyString(string sourceString, string[] stringsToSearchFor, out int foundPos, out int foundStringIndex, int offset = 0)
        {
            foundPos = int.MaxValue;
            foundStringIndex = -1;
            
            if (offset == -1)
                return false;
            
            for (var i = 0; i < stringsToSearchFor.Length; i++)
            {
                int currentFoundPos = sourceString.IndexOf(stringsToSearchFor[i], offset, StringComparison.Ordinal);
                if (currentFoundPos == -1)
                    continue;

                if (currentFoundPos < foundPos)
                {
                    foundPos = currentFoundPos;
                    foundStringIndex = i;
                }
            }

            return foundStringIndex > -1;
        }

        public static string ParsePropertiesInClass(string source, string scopeName)
        {
            string result = "";
            
            int propertyTokenPos = source.IndexOf(PropertyToken, StringComparison.Ordinal);
            while (propertyTokenPos != -1)
            {
                int endOfPropTokenLine = source.IndexOfAny(NewLineChars, propertyTokenPos);

                int beginOfMetadata = propertyTokenPos + PropertyToken.Length + 1;
                int endOfMetadata = source.IndexOf(')', propertyTokenPos);
                PropertyMetadata propertyMetadata = ParsePropertyMetadata(source.Substring(beginOfMetadata, endOfMetadata - beginOfMetadata));
                if (!propertyMetadata.IsValid())
                {
                    Console.WriteLine("Metadata is invalid! Maybe only setter or getter was defined. Currently only both or none are supported!");
                    return "";
                }

                if (endOfPropTokenLine != -1)
                {
                    int startOfPropertyDecl = FindFirstNotOf(source, " \n\r\t", endOfPropTokenLine);
                    int endOfPropertyDecl = source.IndexOfAny(NewLineChars, startOfPropertyDecl);

                    string propertyDecl = source.Substring(startOfPropertyDecl, endOfPropertyDecl - startOfPropertyDecl);

                    int firstSpacePos = propertyDecl.IndexOf(' ');
                    int endOfPropName = propertyDecl.IndexOfAny("={;[".ToCharArray());
                    if (endOfPropName == -1)
                    {
                        Console.WriteLine("ERROR: Couldn't find end of Property declaration! Abort...");
                        return "";
                    }

                    string propName = RemoveWhitespace(propertyDecl.Substring(firstSpacePos + 1, (endOfPropName - 1) - firstSpacePos));
                    if (propName == (PropertyToken + "()"))
                        return "";

                    if(propertyMetadata.DisplayName.Length > 0)
                        result += "\n\t\t\t.property(\"" + propertyMetadata.DisplayName + "\", &" + scopeName + "::";
                    else
                        result += "\n\t\t\t.property(\"" + propName + "\", &" + scopeName + "::";
                    
                    if (propertyMetadata.GetterFunctionName.Length > 0)
                    {
                        result += propertyMetadata.GetterFunctionName + ", &" + scopeName + "::" +propertyMetadata.SetterFunctionName + ')';
                    }
                    else
                    {
                        result += propName + ')';
                    }

                    result += propertyMetadata.MetadataRegistrationString;
                }
                
                propertyTokenPos = source.IndexOf(PropertyToken, endOfPropTokenLine, StringComparison.Ordinal);
            }
            
            int functionTokenPos = source.IndexOf(FunctionToken, StringComparison.Ordinal);
            while (functionTokenPos != -1)
            {
                int endOfPropTokenLine = source.IndexOfAny(NewLineChars, functionTokenPos);

                int beginOfMetadata = functionTokenPos + FunctionToken.Length + 1;
                int endOfMetadata = source.IndexOf(')', functionTokenPos);
                PropertyMetadata propertyMetadata = ParsePropertyMetadata(source.Substring(beginOfMetadata, endOfMetadata - beginOfMetadata), true);
                if (!propertyMetadata.IsValid())
                {
                    Console.WriteLine("Metadata is invalid! Maybe only setter or getter was defined. Currently only both or none are supported!");
                    return "";
                }

                if (endOfPropTokenLine != -1)
                {
                    int startOfPropertyDecl = FindFirstNotOf(source, " \n\r\t", endOfPropTokenLine);
                    int endOfPropertyDecl = source.IndexOfAny(NewLineChars, startOfPropertyDecl);

                    string propertyDecl = source.Substring(startOfPropertyDecl, endOfPropertyDecl - startOfPropertyDecl);

                    int firstSpacePos = propertyDecl.IndexOf(' ');
                    int endOfPropName = propertyDecl.IndexOfAny("(={;[".ToCharArray());
                    if (endOfPropName == -1)
                    {
                        Console.WriteLine("ERROR: Couldn't find end of Function declaration! Abort...");
                        return "";
                    }

                    string propName = RemoveWhitespace(propertyDecl.Substring(firstSpacePos + 1, (endOfPropName - 1) - firstSpacePos));
                    if (propName == (FunctionToken + "()"))
                        return "";

                    if(propertyMetadata.DisplayName.Length > 0)
                        result += "\n\t\t\t.method(\"" + propertyMetadata.DisplayName + "\", &" + scopeName + "::";
                    else
                        result += "\n\t\t\t.method(\"" + propName + "\", &" + scopeName + "::";
                    
                    result += propName + ')';
                    
                    result += propertyMetadata.MetadataRegistrationString;
                }
                
                functionTokenPos = source.IndexOf(FunctionToken, endOfPropTokenLine, StringComparison.Ordinal);
            }
            
            result += ';';
            
            return result;
        }

        class PropertyMetadata
        {
            public string MetadataRegistrationString;
            public string GetterFunctionName;
            public string SetterFunctionName;
            public string DisplayName;

            public PropertyMetadata()
            {
                GetterFunctionName = "";
                SetterFunctionName = "";
                MetadataRegistrationString = "";
                DisplayName = "";
            }

            public bool IsValid()
            {
                return ((GetterFunctionName.Length == 0 && SetterFunctionName.Length == 0) ||
                        (GetterFunctionName.Length > 0 && SetterFunctionName.Length > 0));
            }
        }

        static PropertyMetadata ParsePropertyMetadata(string source, bool bIsFunction = false)
        {
            if (source.Length == 0)
                return new PropertyMetadata();
            
            string strippedSource = RemoveWhitespace(source);
            int currentMetadataPos = 0;

            PropertyMetadata metadata = new PropertyMetadata();

            while (currentMetadataPos != -1)
            {
                int endOfCurrMetadata = strippedSource.IndexOf(',', currentMetadataPos + 1);
                string currentMetadata = strippedSource.Substring(currentMetadataPos, (endOfCurrMetadata != -1 ? endOfCurrMetadata : strippedSource.Length) - currentMetadataPos);
                if (currentMetadata.Length > 0)
                {
                    int equalSignPos = currentMetadata.IndexOf('=');
                    if (equalSignPos != -1)
                    {
                        string metaDataKey = currentMetadata.Substring(0, equalSignPos);
                        string metaDataValue = currentMetadata.Substring(equalSignPos + 1);

                        if (!bIsFunction && metaDataKey == "Getter")
                            metadata.GetterFunctionName = metaDataValue;
                        else if (!bIsFunction && metaDataKey == "Setter")
                            metadata.SetterFunctionName = metaDataValue;
                        else if (!bIsFunction && metaDataKey == "DisplayName")
                            metadata.DisplayName = metaDataValue;
                        else
                        {
                            if (metadata.MetadataRegistrationString.Length > 0)
                                metadata.MetadataRegistrationString += ", ";
                            else
                                metadata.MetadataRegistrationString += "\n\t\t\t\t(";
                            
                            metadata.MetadataRegistrationString += "metadata(\"" + metaDataKey + "\", " + metaDataValue + ")";
                        }
                    }
                    else
                    {
                        if (metadata.MetadataRegistrationString.Length > 0)
                            metadata.MetadataRegistrationString += ", ";
                        else
                            metadata.MetadataRegistrationString += "\n\t\t\t\t(";

                        metadata.MetadataRegistrationString += "metadata(\"" + currentMetadata + "\", true)";
                    }
                }

                currentMetadataPos = endOfCurrMetadata != -1 ? endOfCurrMetadata + 1 : endOfCurrMetadata;
            }

            if (metadata.MetadataRegistrationString.Length > 0)
                metadata.MetadataRegistrationString += ')';

            return metadata;
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

        public static string RemoveWhitespace(string input)
        {
            return new string(input.ToCharArray().Where(c => !Char.IsWhiteSpace(c)).ToArray());
        }

        public static int FindFirstNotOf(string source, string chars, int offset = 0)
        {
            if (source.Length == 0 || chars.Length == 0 || offset >= source.Length)
                return -1;

            for (int i = offset; i < source.Length; i++)
            {
                if (chars.IndexOf(source[i]) == -1)
                    return i;
            }

            return -1;
        }

        public static int FindClosingBracket(string source, int openingBracketPos)
        {
            char openBracket = '{';
            char closeBracket = '}';
            
            int currentOpeningBracketPos = openingBracketPos;
            int potentialClosingBracket = source.IndexOf(closeBracket, openingBracketPos);
            bool bFoundClosingBracket = false;
            while (potentialClosingBracket != -1 && !bFoundClosingBracket)
            {
                int nextOpenBracket = source.IndexOf(openBracket, currentOpeningBracketPos + 1);
                if (nextOpenBracket != -1)
                {
                    if (nextOpenBracket > potentialClosingBracket)
                    {
                        bFoundClosingBracket = true;
                    }
                    else
                    {
                        currentOpeningBracketPos = source.IndexOf(openBracket, nextOpenBracket - 1);
                        potentialClosingBracket = source.IndexOf(closeBracket, potentialClosingBracket + 1);
                    }
                }
                else
                {
                    bFoundClosingBracket = true;
                }
            }

            return bFoundClosingBracket ? potentialClosingBracket : -1;
        }
        
        public static string ExtractDisplayName(string metadataString, string structName)
        {
            // Look for DisplayName="SomeName" in metadata
            string displayNameToken = "DisplayName=\"";
            int displayNamePos = metadataString.IndexOf(displayNameToken);
            if (displayNamePos != -1)
            {
                int startPos = displayNamePos + displayNameToken.Length;
                int endPos = metadataString.IndexOf("\"", startPos);
                if (endPos != -1)
                {
                    return metadataString.Substring(startPos, endPos - startPos);
                }
            }
            
            // If no explicit DisplayName, generate from struct name
            if (structName.EndsWith("Component"))
            {
                return structName.Substring(0, structName.Length - "Component".Length);
            }
            
            return structName;
        }
        
        static void ParseMemberDeclaration(string memberDecl, out string memberType, out string memberName)
        {
            // Simple parsing - find the last space to separate type from name
            int lastSpacePos = memberDecl.LastIndexOf(' ');
            if (lastSpacePos != -1)
            {
                memberType = memberDecl.Substring(0, lastSpacePos).Trim();
                memberName = memberDecl.Substring(lastSpacePos + 1).Trim();
            }
            else
            {
                memberType = "";
                memberName = memberDecl.Trim();
            }
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
        
        static void GenerateComponentSectionInstantiations()
        {
            Console.WriteLine("Generating template instantiations for CreateComponentSection...");
            
            string instantiationsContent = "// Auto-generated template instantiations for CreateComponentSection\n\n";
            instantiationsContent += "#include \"Panels/ComponentSectionHelper.h\"\n";
            
            // Read Haketon component registry entries if they exist
            string haketonRegistryPath = SolutionDir + "Haketon\\src\\GeneratedFiles\\HaketonComponentRegistryEntries.gen.txt";
            if (File.Exists(haketonRegistryPath))
            {
                string haketonRegistryEntries = File.ReadAllText(haketonRegistryPath);
                
                // Extract includes from Haketon registry entries
                string[] lines = haketonRegistryEntries.Split('\n');
                foreach (string line in lines)
                {
                    if (line.Trim().StartsWith("#include"))
                    {
                        instantiationsContent += line + "\n";
                    }
                }
            }
            
            // Add includes for discovered components (remove duplicates)
            var includedPaths = new HashSet<string>();
            foreach (var component in DiscoveredComponents)
            {
                if (!includedPaths.Contains(component.IncludePath))
                {
                    instantiationsContent += $"#include \"{component.IncludePath}\"\n";
                    includedPaths.Add(component.IncludePath);
                }
            }
            
            instantiationsContent += "\nnamespace Haketon\n{\n";
            
            // Add Haketon template instantiations
            if (File.Exists(haketonRegistryPath))
            {
                string haketonRegistryEntries = File.ReadAllText(haketonRegistryPath);
                
                // Extract template instantiations (lines after "// Template instantiations")
                string[] lines = haketonRegistryEntries.Split('\n');
                bool inInstantiationSection = false;
                foreach (string line in lines)
                {
                    if (line.Contains("// Template instantiations"))
                    {
                        inInstantiationSection = true;
                        instantiationsContent += "\t// Components from Haketon project\n";
                        continue;
                    }
                    if (inInstantiationSection && !string.IsNullOrWhiteSpace(line) && line.Contains("template void"))
                    {
                        instantiationsContent += line + "\n";
                    }
                }
                instantiationsContent += "\n";
            }
            
            // Discovered components from HaketonEditor
            instantiationsContent += "\t// Discovered components from HaketonEditor project\n";
            foreach (var component in DiscoveredComponents)
            {
                instantiationsContent += $"\ttemplate void CreateComponentSection<{component.Name}>(Entity entity, bool isRemovable);\n";
            }
            
            instantiationsContent += "}\n";
            
            string instantiationsPath = OutputDir + "ComponentSectionInstantiations.gen.cpp";
            File.WriteAllText(instantiationsPath, instantiationsContent);
            Console.WriteLine($"Generated template instantiations: {instantiationsPath}");
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