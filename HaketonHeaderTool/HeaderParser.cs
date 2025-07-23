using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace HaketonHeaderTool
{
    // Enhanced header parser with better error handling and line tracking
    public static class HeaderParser
    {
        public static bool GenerateHeaderForHeader(Program.HeaderFileInfo headerFileInfo)
        {
            try
            {
                if (!File.Exists(headerFileInfo.FullPath))
                {
                    throw new FileProcessingException($"Header file does not exist", headerFileInfo.FullPath);
                }

                Logger.Debug($"Processing header file: {headerFileInfo.FullPath}");
                
                string fileContent = File.ReadAllText(headerFileInfo.FullPath);
                if (string.IsNullOrEmpty(fileContent))
                {
                    Logger.Warning($"Header file is empty: {headerFileInfo.FullPath}");
                    return false;
                }

                // Create source tracker for line number reporting
                var sourceTracker = new SourceTracker(fileContent, headerFileInfo.FullPath);
                string processedContent = RemoveComments(fileContent);
                
                var registrationBuilder = new RegistrationBuilder();
                var discoveredComponents = new List<Program.ComponentInfo>();
                
                // Parse enums
                ParseEnums(processedContent, sourceTracker, registrationBuilder, headerFileInfo.FullPath);
                
                // Parse structs and classes
                ParseStructs(processedContent, sourceTracker, registrationBuilder, headerFileInfo, discoveredComponents);
                
                // Generate output if we found anything
                if (registrationBuilder.HasContent)
                {
                    GenerateOutputFile(headerFileInfo, registrationBuilder.ToString(), processedContent);
                    
                    // Add discovered components to the global list
                    foreach (var component in discoveredComponents)
                    {
                        Program.DiscoveredComponents.Add(component);
                    }
                    
                    // Track this function for the master header
                    Program.GeneratedFunctions.Add("Register" + headerFileInfo.FileName + "Types");
                    
                    Logger.Info($"Successfully processed {headerFileInfo.FileName}.h - found {discoveredComponents.Count} components");
                }
                else
                {
                    Logger.Debug($"No reflection tokens found in {headerFileInfo.FileName}.h");
                }
                
                return true;
            }
            catch (HeaderToolException)
            {
                // Re-throw HeaderTool exceptions as-is
                throw;
            }
            catch (Exception ex)
            {
                throw new FileProcessingException($"Failed to process header file", ex, headerFileInfo.FullPath);
            }
        }
        
        private static void ParseEnums(string source, SourceTracker sourceTracker, RegistrationBuilder builder, string fileName)
        {
            int enumTokenPos = source.IndexOf("ENUM", StringComparison.Ordinal);
            while (enumTokenPos != -1)
            {
                try
                {
                    var enumPos = sourceTracker.GetPositionAt(enumTokenPos);
                    Logger.Debug($"Found ENUM token at {enumPos}");
                    
                    int endOfLinePos = source.IndexOfAny(new[] { '\r', '\n' }, enumTokenPos);
                    if (endOfLinePos == -1)
                    {
                        Logger.Warning($"ENUM declared at end of file, ignoring", fileName, enumPos.LineNumber);
                        break;
                    }
                    
                    int startOfEnumDecl = FindFirstNotOf(source, " \n\r\t", endOfLinePos);
                    int endOfEnumDecl = source.IndexOfAny(new[] { '\r', '\n' }, startOfEnumDecl);
                    if (endOfEnumDecl == -1)
                    {
                        throw new ParseException("Incomplete enum declaration", fileName, enumPos.LineNumber);
                    }
                    
                    string enumDecl = source.Substring(startOfEnumDecl, endOfEnumDecl - startOfEnumDecl);
                    string enumName = ExtractEnumName(enumDecl, fileName, sourceTracker.GetPositionAt(startOfEnumDecl).LineNumber);
                    
                    // Find enum body
                    int openingBracketPos = source.IndexOf('{', endOfEnumDecl);
                    int endOfEnumPos = FindClosingBracket(source, openingBracketPos);
                    if (endOfEnumPos == -1 || openingBracketPos == -1)
                    {
                        var bracketPos = sourceTracker.GetPositionAt(endOfEnumDecl);
                        throw new ParseException($"No closing bracket found for enum '{enumName}'", fileName, bracketPos.LineNumber);
                    }
                    
                    // Generate enum registration
                    string enumBody = source.Substring(openingBracketPos + 1, endOfEnumPos - (openingBracketPos + 1));
                    builder.AddEnumRegistration(enumName, enumBody);
                    
                    Logger.Debug($"Successfully parsed enum: {enumName}");
                    
                    // Find next enum
                    enumTokenPos = source.IndexOf("ENUM", endOfEnumDecl, StringComparison.Ordinal);
                }
                catch (Exception ex) when (!(ex is HeaderToolException))
                {
                    var pos = sourceTracker.GetPositionAt(enumTokenPos);
                    throw new ParseException($"Error parsing enum: {ex.Message}", fileName, pos.LineNumber);
                }
            }
        }
        
        private static void ParseStructs(string source, SourceTracker sourceTracker, RegistrationBuilder builder, 
            Program.HeaderFileInfo headerFileInfo, List<Program.ComponentInfo> discoveredComponents)
        {
            int structTokenPos = source.IndexOf("STRUCT", StringComparison.Ordinal);
            while (structTokenPos != -1)
            {
                try
                {
                    var structPos = sourceTracker.GetPositionAt(structTokenPos);
                    Logger.Debug($"Found STRUCT token at {structPos}");
                    
                    // Parse metadata
                    string metadataString = ExtractMetadata(source, structTokenPos, "STRUCT");
                    
                    int endOfLinePos = source.IndexOfAny(new[] { '\r', '\n' }, structTokenPos);
                    if (endOfLinePos == -1)
                    {
                        Logger.Warning($"STRUCT declared at end of file, ignoring", headerFileInfo.FullPath, structPos.LineNumber);
                        break;
                    }
                    
                    int startOfStructDecl = FindFirstNotOf(source, " \n\r\t", endOfLinePos);
                    int endOfStructDecl = source.IndexOfAny(new[] { '\r', '\n' }, startOfStructDecl);
                    
                    string structDecl = source.Substring(startOfStructDecl, endOfStructDecl - startOfStructDecl);
                    string structName = ExtractStructName(structDecl, headerFileInfo.FullPath, sourceTracker.GetPositionAt(startOfStructDecl).LineNumber);
                    
                    // Find struct body
                    int openingBracketPos = source.IndexOf('{', endOfStructDecl);
                    int endOfStructPos = FindClosingBracket(source, openingBracketPos);
                    if (endOfStructPos == -1)
                    {
                        var bracketPos = sourceTracker.GetPositionAt(endOfStructDecl);
                        throw new ParseException($"No closing bracket found for struct '{structName}'", headerFileInfo.FullPath, bracketPos.LineNumber);
                    }
                    
                    string structBody = source.Substring(endOfStructDecl, endOfStructPos - endOfStructDecl);
                    
                    // Check if this is a component
                    if (IsComponent(structBody, structDecl))
                    {
                        string displayName = ExtractDisplayName(metadataString, structName);
                        bool isRemovable = !metadataString.Contains("NonRemovable");
                        string includePath = headerFileInfo.IncludeDir + headerFileInfo.FileNameWithExt;
                        
                        var componentInfo = new Program.ComponentInfo(structName, displayName, isRemovable, includePath);
                        discoveredComponents.Add(componentInfo);
                        
                        Logger.Info($"Found component: {structName} (Display: '{displayName}', Removable: {isRemovable})");
                    }
                    
                    // Generate class registration
                    string propertyRegistration = ParsePropertiesInClass(structBody, structName, headerFileInfo.FullPath, sourceTracker);
                    builder.AddClassRegistration(structName, propertyRegistration);
                    
                    Logger.Debug($"Successfully parsed struct: {structName}");
                    
                    // Find next struct
                    structTokenPos = source.IndexOf("STRUCT", endOfStructDecl, StringComparison.Ordinal);
                }
                catch (Exception ex) when (!(ex is HeaderToolException))
                {
                    var pos = sourceTracker.GetPositionAt(structTokenPos);
                    throw new ParseException($"Error parsing struct: {ex.Message}", headerFileInfo.FullPath, pos.LineNumber);
                }
            }
        }
        
        private static string ExtractMetadata(string source, int tokenPos, string token)
        {
            int beginOfMetadata = tokenPos + token.Length;
            if (beginOfMetadata < source.Length && source[beginOfMetadata] == '(')
            {
                int endOfMetadata = source.IndexOf(')', beginOfMetadata);
                if (endOfMetadata != -1)
                {
                    return source.Substring(beginOfMetadata + 1, endOfMetadata - beginOfMetadata - 1);
                }
            }
            return "";
        }
        
        private static string ExtractEnumName(string enumDecl, string fileName, int lineNumber)
        {
            if (enumDecl.StartsWith("enum class "))
            {
                return enumDecl.Substring(11).Trim();
            }
            else if (enumDecl.StartsWith("enum "))
            {
                return enumDecl.Substring(5).Trim();
            }
            else
            {
                throw new ParseException("ENUM specifier used on non-enum type", fileName, lineNumber);
            }
        }
        
        private static string ExtractStructName(string structDecl, string fileName, int lineNumber)
        {
            if (structDecl.StartsWith("struct "))
            {
                int spacePos = structDecl.IndexOf(' ', 7);
                return spacePos != -1 ? structDecl.Substring(7, spacePos - 7) : structDecl.Substring(7);
            }
            else if (structDecl.StartsWith("class "))
            {
                int spacePos = structDecl.IndexOf(' ', 6);
                return spacePos != -1 ? structDecl.Substring(6, spacePos - 6) : structDecl.Substring(6);
            }
            else
            {
                throw new ParseException("STRUCT specifier used on non-struct/class type", fileName, lineNumber);
            }
        }
        
        private static bool IsComponent(string structBody, string structDecl)
        {
            return structBody.Contains("RTTR_ENABLE(Component)") || 
                   structDecl.Contains(": Component") || 
                   structDecl.Contains(":Component");
        }
        
        private static string ExtractDisplayName(string metadataString, string structName)
        {
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
            
            // Generate from struct name
            return structName.EndsWith("Component") ? structName.Substring(0, structName.Length - "Component".Length) : structName;
        }
        
        // Placeholder methods - would need full implementation
        private static string RemoveComments(string source) => Program.RemoveComments(source);
        private static int FindFirstNotOf(string source, string chars, int offset) => Program.FindFirstNotOf(source, chars, offset);
        private static int FindClosingBracket(string source, int openPos) => Program.FindClosingBracket(source, openPos);
        private static string ParsePropertiesInClass(string source, string scopeName, string fileName, SourceTracker tracker) => Program.ParsePropertiesInClass(source, scopeName);
        
        private static void GenerateOutputFile(Program.HeaderFileInfo headerFileInfo, string registrationString, string source)
        {
            string additionalIncludes = ScanFileForAdditionalIncludes(source, headerFileInfo.FileName);
            string includeString = $"#include \"{headerFileInfo.IncludeDir}{headerFileInfo.FileNameWithExt}\"\r\n{additionalIncludes}";
            string genFileString = $"#include \"hkpch.h\"\n{includeString}\n\n#include <rttr/registration>\n\n\nnamespace Haketon\n{{\n\tvoid Register{headerFileInfo.FileName}Types()\n\t{{\n\t\tusing namespace rttr;\n\t\tstatic bool registered = false;\n\t\tif (registered) return;\n\t\tregistered = true;\n\t\t\n{registrationString}\t}}\n}}";
            
            Directory.CreateDirectory(Program.OutputDir);
            string outputPath = Program.OutputDir + headerFileInfo.FileName + ".gen.cpp";
            File.WriteAllText(outputPath, genFileString);
            
            Logger.Debug($"Generated output file: {outputPath}");
        }
        
        private static string ScanFileForAdditionalIncludes(string source, string headerFileName)
        {
            string includeString = "";
            
            const string classString = "class ";
            const string structString = "struct ";
            
            // Are there class forward declarations?
            var forwardDeclaredClasses = new List<string>();
            
            string[] typesToSearchFor = { "class ", "struct " };
            int currentTypePos;
            int currentTypeIndex;
            int currentOffset = 0;
            
            while (FirstIndexOfAnyString(source, typesToSearchFor, out currentTypePos, out currentTypeIndex, currentOffset))
            {
                int endOfDeclPos = source.IndexOf(';', currentTypePos);
                int endOfLinePos = source.IndexOfAny(new[] { '\r', '\n' }, currentTypePos);
                if (endOfDeclPos > -1 && ((endOfDeclPos <= endOfLinePos) || endOfLinePos == -1))
                {
                    string typeDeclaration = source.Substring(currentTypePos + typesToSearchFor[currentTypeIndex].Length, endOfDeclPos - (currentTypePos + typesToSearchFor[currentTypeIndex].Length));
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
                foreach (string file in Program.FilesToScan)
                {
                    if (file.Contains(forwardDeclaredClass + ".h"))
                    {
                        string fileSource = File.ReadAllText(file);
                        while (FirstIndexOfAnyString(fileSource, typesToSearchFor, out currentTypePos, out currentTypeIndex, currentOffset))
                        {
                            int endOfStructPos = Program.FindClosingBracket(fileSource, fileSource.IndexOf('{', currentTypePos));
                            int endOfDecl = fileSource.IndexOf(' ', currentTypePos + typesToSearchFor[currentTypeIndex].Length);
                            if(endOfDecl == -1)
                                endOfDecl = fileSource.IndexOfAny(new[] { '\r', '\n' }, currentTypePos);
                            
                            if (endOfStructPos != -1 && endOfDecl != -1)
                            {
                                string typeDeclaration = RemoveWhitespace(fileSource.Substring(currentTypePos + typesToSearchFor[currentTypeIndex].Length, endOfDecl - (currentTypePos + typesToSearchFor[currentTypeIndex].Length)));
                                if (typeDeclaration == forwardDeclaredClass)
                                {
                                    bFoundClass = true;
                                    includeString += "#include \"" + file.Replace(Program.ProjectSrcDir, "") + "\"\r\n";
                                }
                                    
                            }

                            currentOffset = endOfDecl;
                        }
                    }
                }
                
                if(!bFoundClass)
                    Logger.Warning($"Class {forwardDeclaredClass} not found in files... Maybe it is in file not called after the class?");
            }

            return includeString;
        }
        
        private static bool FirstIndexOfAnyString(string sourceString, string[] stringsToSearchFor, out int foundPos, out int foundStringIndex, int offset = 0)
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
        
        private static string RemoveWhitespace(string input)
        {
            return new string(input.ToCharArray().Where(c => !char.IsWhiteSpace(c)).ToArray());
        }
    }
    
    // Helper class to build registration strings
    public class RegistrationBuilder
    {
        private readonly System.Text.StringBuilder _content = new System.Text.StringBuilder();
        
        public bool HasContent => _content.Length > 0;
        
        public void AddEnumRegistration(string enumName, string enumBody)
        {
            _content.AppendLine($"\n\t\tregistration::enumeration<{enumName}>(\"{enumName}\")");
            _content.Append("\t\t\t(");
            
            // Process enum values
            string[] enumValues = enumBody.Replace(" ", "").Replace("\n", "").Replace("\r", "").Split(',');
            for (int i = 0; i < enumValues.Length; i++)
            {
                string enumValueName = enumValues[i];
                int equalPos = enumValueName.IndexOf('=');
                if (equalPos != -1)
                    enumValueName = enumValueName.Substring(0, equalPos);
                
                if (i > 0) _content.Append(",");
                _content.Append($"\n\t\t\t\tvalue(\"{enumValueName}\",\t{enumName}::{enumValueName})");
            }
            
            _content.AppendLine("\n\t\t\t);");
        }
        
        public void AddClassRegistration(string className, string propertyRegistration)
        {
            _content.AppendLine($"\n\t\tregistration::class_<{className}>(\"{className}\")");
            _content.Append($"\t\t\t.constructor(){propertyRegistration}");
        }
        
        public override string ToString()
        {
            return _content.ToString();
        }
    }
}