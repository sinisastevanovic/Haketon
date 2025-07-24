using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace HaketonHeaderTool
{
    // Enhanced header parser with proper tokenizer -> parser -> generator architecture
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

                // Phase 1: Tokenization
                Logger.Debug($"Tokenizing {headerFileInfo.FileName}.h");
                string cleanContent = Program.RemoveComments(fileContent);
                var tokenizer = new Tokenizer(cleanContent, headerFileInfo.FullPath);
                var tokens = tokenizer.Tokenize();

                if (!tokenizer.FoundRelevantToken)
                {
                    Logger.Debug($"Skipping file because there are no relevant tokens...");
                    return false;
                }
                
                Logger.Debug($"Generated {tokens.Count} tokens");
                
                // Phase 2: Parsing
                Logger.Debug($"Parsing {headerFileInfo.FileName}.h");
                var parser = new Parser(tokens);
                var fileNode = parser.ParseFile(headerFileInfo.FileName);
                
                Logger.Debug($"Parsed {fileNode.Children.Count} top-level declarations");
                
                // Phase 3: Code Generation
                Logger.Debug($"Generating code for {headerFileInfo.FileName}.h");
                var generator = new CodeGenerator(headerFileInfo);
                var generatedFile = generator.GenerateRegistrationFile(fileNode);
                
                if (generatedFile != null)
                {
                    // Write the generated file
                    Directory.CreateDirectory(Program.OutputDir);
                    string outputPath = Path.Combine(Program.OutputDir, generatedFile.FileName);
                    File.WriteAllText(outputPath, generatedFile.Content);
                    
                    Logger.Debug($"Generated output file: {outputPath}");
                    return true;
                }
                else
                {
                    Logger.Debug($"No reflection tokens found in {headerFileInfo.FileName}.h");
                    return false;
                }
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