using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace HaketonHeaderTool
{
    public class CodeGenerator
    {
        private readonly HeaderFileInfo _headerFileInfo;
        private readonly List<ForwardDeclarationNode> _forwardDeclarations = new List<ForwardDeclarationNode>();
        
        public CodeGenerator(HeaderFileInfo headerFileInfo)
        {
            _headerFileInfo = headerFileInfo ?? throw new ArgumentNullException(nameof(headerFileInfo));
        }
        
        public GeneratedFile GenerateRegistrationFile(FileNode fileNode)
        {
            if (fileNode == null)
                throw new ArgumentNullException(nameof(fileNode));
            
            var registrationBuilder = new StringBuilder();
            var componentsFound = new List<ComponentInfo>();
            var functionsGenerated = new List<string>();
            
            // Collect forward declarations
            _forwardDeclarations.Clear();
            CollectForwardDeclarations(fileNode);
            
            // Generate registration code for each top-level declaration
            foreach (var child in fileNode.Children)
            {
                switch (child)
                {
                    case EnumNode enumNode:
                        GenerateEnumRegistration(enumNode, registrationBuilder);
                        break;
                        
                    case StructNode structNode:
                        GenerateStructRegistration(structNode, registrationBuilder, componentsFound);
                        break;
                }
            }
            
            if (registrationBuilder.Length == 0)
            {
                Logger.Debug($"No reflection tokens found in {_headerFileInfo.FileName}.h");
                return null;
            }
            
            // Generate the complete file
            var fileContent = GenerateCompleteFile(registrationBuilder.ToString());
            var fileName = $"{_headerFileInfo.FileName}.gen.cpp";
            var generatedFile = new GeneratedFile(fileName, fileContent);
            
            // Add additional includes for forward declared classes
            generatedFile.AdditionalIncludes.AddRange(ResolveForwardDeclarationIncludes());
            
            // Update global state (for compatibility with existing system)
            foreach (var component in componentsFound)
            {
                ProjectConfiguration.DiscoveredComponents.Add(component);
            }
            
            string functionName = $"Register{_headerFileInfo.FileName}Types";
            ProjectConfiguration.GeneratedFunctions.Add(functionName);
            
            Logger.Info($"Successfully generated {fileName} - found {componentsFound.Count} components");
            
            return generatedFile;
        }
        
        private void CollectForwardDeclarations(ASTNode node)
        {
            if (node is ForwardDeclarationNode forwardDecl)
            {
                _forwardDeclarations.Add(forwardDecl);
            }
            else if (node is FileNode fileNode)
            {
                foreach (var child in fileNode.Children)
                {
                    CollectForwardDeclarations(child);
                }
            }
        }
        
        private void GenerateEnumRegistration(EnumNode enumNode, StringBuilder builder)
        {
            Logger.Debug($"Generating enum registration for: {enumNode.Name}");
            
            builder.AppendLine();
            builder.AppendLine($"\t\tregistration::enumeration<{enumNode.Name}>(\"{enumNode.Name}\")");
            builder.Append("\t\t\t(");
            
            for (int i = 0; i < enumNode.Values.Count; i++)
            {
                var enumValue = enumNode.Values[i];
                if (i > 0) builder.Append(",");
                builder.AppendLine();
                builder.Append($"\t\t\t\tvalue(\"{enumValue.Name}\",\t{enumNode.Name}::{enumValue.Name})");
            }
            
            builder.AppendLine();
            builder.AppendLine("\t\t\t);");
        }
        
        private void GenerateStructRegistration(StructNode structNode, StringBuilder builder, List<ComponentInfo> componentsFound)
        {
            Logger.Debug($"Generating struct registration for: {structNode.Name}");
            
            // Check if this is a component and add to component list
            if (structNode.IsComponent || IsComponent(structNode))
            {
                var componentInfo = CreateComponentInfo(structNode);
                componentsFound.Add(componentInfo);
                Logger.Info($"Found component: {structNode.Name} (Display: '{componentInfo.DisplayName}', Removable: {componentInfo.IsRemovable})");
            }
            
            // Generate class registration
            builder.AppendLine();
            builder.AppendLine($"\t\tregistration::class_<{structNode.Name}>(\"{structNode.Name}\")");
            builder.Append("\t\t\t.constructor()");
            
            // Generate property registrations
            foreach (var property in structNode.Properties)
            {
                GeneratePropertyRegistration(property, structNode.Name, builder);
            }
            
            // Generate function registrations
            foreach (var function in structNode.Functions)
            {
                GenerateFunctionRegistration(function, structNode.Name, builder);
            }
            
            builder.AppendLine(";");
        }
        
        private void GeneratePropertyRegistration(PropertyNode property, string className, StringBuilder builder)
        {
            string displayName = property.Metadata?.GetProperty("DisplayName", property.Name) ?? property.Name;
            
            builder.AppendLine();
            
            // Check for custom getter/setter
            string getter = property.Metadata?.GetProperty("Getter");
            string setter = property.Metadata?.GetProperty("Setter");
            
            if (!string.IsNullOrEmpty(getter) && !string.IsNullOrEmpty(setter))
            {
                builder.Append($"\t\t\t.property(\"{displayName}\", &{className}::{getter}, &{className}::{setter})");
            }
            else
            {
                builder.Append($"\t\t\t.property(\"{displayName}\", &{className}::{property.Name})");
            }
            
            // Add metadata if present
            GenerateMetadata(property.Metadata, builder);
        }
        
        private void GenerateFunctionRegistration(FunctionNode function, string className, StringBuilder builder)
        {
            string displayName = function.Metadata?.GetProperty("DisplayName", function.Name) ?? function.Name;
            
            builder.AppendLine();
            builder.Append($"\t\t\t.method(\"{displayName}\", &{className}::{function.Name})");
            
            // Add metadata if present
            GenerateMetadata(function.Metadata, builder);
        }
        
        private bool IsAlreadyQuoted(string value)
        {
            if (string.IsNullOrEmpty(value))
                return false;
                
            return (value.StartsWith("\"") && value.EndsWith("\"")) ||
                   (value.StartsWith("'") && value.EndsWith("'"));
        }
        
        private bool IsNumericValue(string value)
        {
            if (string.IsNullOrEmpty(value))
                return false;
                
            // Check for integer literals
            if (int.TryParse(value, out _))
                return true;
                
            // Check for float literals
            if (float.TryParse(value, out _))
                return true;
                
            // Check for common numeric suffixes
            return value.EndsWith("f") || value.EndsWith("F") ||
                   value.EndsWith("l") || value.EndsWith("L") ||
                   value.EndsWith("u") || value.EndsWith("U");
        }
        
        private void GenerateMetadata(Metadata metadata, StringBuilder builder)
        {
            if (metadata == null || metadata.Properties.Count == 0)
                return;
            
            var metadataEntries = new List<string>();
            
            foreach (var kvp in metadata.Properties)
            {
                // Skip special properties that are handled elsewhere
                if (kvp.Key == "DisplayName" || kvp.Key == "Getter" || kvp.Key == "Setter")
                    continue;
                
                // Always quote EditCondition and Tooltip values as strings
                if (kvp.Key == "EditCondition" || kvp.Key == "Tooltip")
                {
                    metadataEntries.Add($"metadata(\"{kvp.Key}\", \"{kvp.Value}\")");
                }
                else if (kvp.Value == "true" || string.IsNullOrEmpty(kvp.Value))
                {
                    metadataEntries.Add($"metadata(\"{kvp.Key}\", true)");
                }
                else if (kvp.Value == "false")
                {
                    metadataEntries.Add($"metadata(\"{kvp.Key}\", false)");
                }
                else if (IsAlreadyQuoted(kvp.Value) || IsNumericValue(kvp.Value))
                {
                    // Value is already quoted or is a numeric literal - use as-is
                    metadataEntries.Add($"metadata(\"{kvp.Key}\", {kvp.Value})");
                }
                else
                {
                    // Quote the value as a string literal
                    metadataEntries.Add($"metadata(\"{kvp.Key}\", \"{kvp.Value}\")");
                }
            }
            
            if (metadataEntries.Count > 0)
            {
                builder.AppendLine();
                builder.Append("\t\t\t\t(");
                for (int i = 0; i < metadataEntries.Count; i++)
                {
                    if (i > 0) builder.Append(", ");
                    builder.Append(metadataEntries[i]);
                }
                builder.Append(")");
            }
        }
        
        private bool IsComponent(StructNode structNode)
        {
            return structNode.IsComponent || 
                   structNode.BaseClass == "Component" ||
                   structNode.Name.EndsWith("Component");
        }
        
        private ComponentInfo CreateComponentInfo(StructNode structNode)
        {
            string displayName = ExtractDisplayName(structNode);
            bool isRemovable = !structNode.Metadata?.GetBoolProperty("NonRemovable") ?? true;
            string includePath = _headerFileInfo.IncludeDir + _headerFileInfo.FileNameWithExt;
            
            return new ComponentInfo(structNode.Name, displayName, isRemovable, includePath);
        }
        
        private string ExtractDisplayName(StructNode structNode)
        {
            // First check metadata
            string displayName = structNode.Metadata?.GetProperty("DisplayName");
            if (!string.IsNullOrEmpty(displayName))
                return displayName;
            
            // Generate from struct name
            if (structNode.Name.EndsWith("Component"))
            {
                return structNode.Name.Substring(0, structNode.Name.Length - "Component".Length);
            }
            
            return structNode.Name;
        }
        
        private List<string> ResolveForwardDeclarationIncludes()
        {
            var includes = new List<string>();
            
            foreach (var forwardDecl in _forwardDeclarations)
            {
                string includePath = FindIncludePathForClass(forwardDecl.Name);
                if (!string.IsNullOrEmpty(includePath))
                {
                    includes.Add($"#include \"{includePath}\"");
                }
                else
                {
                    Logger.Warning($"Could not find include path for forward declared class '{forwardDecl.Name}'");
                }
            }
            
            return includes;
        }
        
        private string FindIncludePathForClass(string className)
        {
            // Search through all files to find the class definition
            if (ProjectConfiguration.FilesToScan == null)
                return null;
            
            foreach (string file in ProjectConfiguration.FilesToScan)
            {
                if (file.Contains(className + ".h"))
                {
                    try
                    {
                        string fileContent = File.ReadAllText(file);
                        
                        // Use the improved comment removal
                        string cleanContent = SourceProcessingUtilities.RemoveComments(fileContent);
                        
                        // Look for class or struct definition
                        if (cleanContent.Contains($"class {className}") || 
                            cleanContent.Contains($"struct {className}"))
                        {
                            return file.Replace(ProjectConfiguration.ProjectSrcDir, "").Replace("\\", "/");
                        }
                    }
                    catch (Exception ex)
                    {
                        Logger.Warning($"Error reading file {file} while searching for {className}: {ex.Message}");
                    }
                }
            }
            
            return null;
        }
        
        private string GenerateCompleteFile(string registrationContent)
        {
            var builder = new StringBuilder();
            
            // Add precompiled header
            builder.AppendLine("#include \"hkpch.h\"");
            
            // Add main include
            builder.AppendLine($"#include \"{_headerFileInfo.IncludeDir}{_headerFileInfo.FileNameWithExt}\"");
            
            // Add additional includes for forward declarations
            var additionalIncludes = ResolveForwardDeclarationIncludes();
            foreach (string include in additionalIncludes)
            {
                builder.AppendLine(include);
            }
            
            // Add RTTR include
            builder.AppendLine();
            builder.AppendLine("#include <rttr/registration>");
            
            // Add namespace and function
            builder.AppendLine();
            builder.AppendLine();
            builder.AppendLine("namespace Haketon");
            builder.AppendLine("{");
            builder.AppendLine($"\tvoid Register{_headerFileInfo.FileName}Types()");
            builder.AppendLine("\t{");
            builder.AppendLine("\t\tusing namespace rttr;");
            builder.AppendLine("\t\tstatic bool registered = false;");
            builder.AppendLine("\t\tif (registered) return;");
            builder.AppendLine("\t\tregistered = true;");
            builder.AppendLine();
            
            // Add registration content
            builder.Append(registrationContent);
            
            // Close function and namespace
            builder.AppendLine("\t}");
            builder.AppendLine("}");
            
            return builder.ToString();
        }
    }
}