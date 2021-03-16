using System;
using System.IO;
using System.Linq;

namespace HaketonHeaderTool
{
    
    // TODO: Add errors and line numbers
    class Program
    {
        private const string StructToken = "STRUCT";
        private const string ClassToken = "CLASS";
        private const string PropertyToken = "PROPERTY";
        private const string EnumToken = "ENUM";
        private const string FunctionToken = "FUNCTION";
        
        const string EngineSrcDir = "Haketon\\src\\";
        const string EditorSrcDir = "HaketonEditor\\src\\";
        const string OutputDir = "HaketonEditor\\src\\GeneratedFiles\\";
        static string SolutionDir = "..\\..\\..\\..\\";
        static string ProjectName = "Haketon";

        private static readonly char[] NewLineChars = new[] {'\r', '\n'};

        class HeaderFileInfo
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
            string[] arguments = Environment.GetCommandLineArgs();
            if (arguments.Length > 2 && Directory.Exists(arguments[1]))
            {
                SolutionDir = arguments[1];
                ProjectName = arguments[2];
            }

            string dirToSearch = SolutionDir + ProjectName + "\\src\\";
            
            if(!Directory.Exists(dirToSearch))
                Console.WriteLine("ERROR: HaketonHeaderTool: Directory {0} does not exist!", dirToSearch);

            Console.WriteLine("HaketonHeaderTool: scanning files for {0}...", ProjectName);
            
            string[] filesToScan = Directory.GetFiles(dirToSearch, "*.h", SearchOption.AllDirectories);
            if (filesToScan.Length == 0)
                Console.WriteLine("HaketonHeaderTool: no files in project {0}. Skipping", ProjectName);
           
            // Delete all previously generated files for current project
            string genFileDir = SolutionDir + OutputDir + ProjectName + "\\";
            if (Directory.Exists(genFileDir))
            {
                string[] existingFiles = Directory.GetFiles(genFileDir, "*.gen.cpp", SearchOption.AllDirectories);
                foreach (string existingFile in existingFiles)
                {
                    File.Delete(existingFile);
                }
            }

            foreach (string file in filesToScan)
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
            }
            
            /*HeaderFileInfo headerFileInfo = new HeaderFileInfo(SolutionDir + EngineSrcDir, "Haketon\\Scene\\", "SceneCamera");
            Console.WriteLine(string.Format("Generating header for {0}", headerFileInfo.FullPath));
            if(GenerateHeaderForHeader(headerFileInfo))
                Console.WriteLine("Success!");
            else
                Console.WriteLine("Failed!");*/
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
                
                registrationString += "\n\n\t\tregistration::class_<" + structName + ">(\"" + structName + "\")\n\t\t\t.constructor()" + ParsePropertiesInClass(structSource, structName);

                // Find next struct
                structTokenPos = fileString.IndexOf(StructToken, endOfStructDecl, StringComparison.Ordinal);
            }

            if (registrationString.Length > 0)
            {
                string includeString = "#include \"" + headerFileInfo.IncludeDir + headerFileInfo.FileNameWithExt + "\""; 
                string genFileString = "#include \"hkpch.h\"\n" + includeString + "\n\n#include <rttr/registration>\n\n\nnamespace Haketon\n{\n\tRTTR_REGISTRATION\n\t{\n\t\tusing namespace rttr;";
                genFileString += registrationString;
                genFileString += "\n\t}\n}";

                string genFileDir = SolutionDir + OutputDir + ProjectName + "\\";
                Directory.CreateDirectory(genFileDir);
                File.WriteAllText(genFileDir + headerFileInfo.FileName + ".gen.cpp", genFileString);

                /*string projFilePath = @"E:\Haketon\HaketonEditor\HaketonEditor.vcxproj";
                var p = Microsoft.Build.Evaluation.ProjectCollection.GlobalProjectCollection.LoadedProjects
                    .FirstOrDefault(pr => pr.FullPath == @"E:\Haketon\HaketonEditor\HaketonEditor.vcxproj");
                if (p == null)
                    p = new Microsoft.Build.Evaluation.Project(@"E:\Haketon\HaketonEditor\HaketonEditor.vcxproj");

                p.AddItem("Folder", @"E:\Haketon\HaketonEditor\src\GeneratedFiles\Haketon");
                p.AddItem("Compile", @"E:\Haketon\HaketonEditor\src\GeneratedFiles\Haketon\Components.gen.cpp");
                p.Save();*/
            }
            
            return true;
        }

        static string ParsePropertiesInClass(string source, string scopeName)
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
                    int endOfPropName = propertyDecl.IndexOfAny("={;".ToCharArray());
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

        static PropertyMetadata ParsePropertyMetadata(string source)
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

                        if (metaDataKey == "Getter")
                            metadata.GetterFunctionName = metaDataValue;
                        else if (metaDataKey == "Setter")
                            metadata.SetterFunctionName = metaDataValue;
                        else if (metaDataKey == "DisplayName")
                            metadata.DisplayName = metaDataValue;
                        else
                        {
                            if (metadata.MetadataRegistrationString.Length > 0)
                                metadata.MetadataRegistrationString += ", ";
                            else
                                metadata.MetadataRegistrationString += "\n\t\t\t\t(";
                            
                            metadata.MetadataRegistrationString += "metadata(\"" + metaDataKey + "\", \"" + metaDataValue + "\")";
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

        static string RemoveComments(string String)
        {
            return Remove(Remove(String, "/*", "*/"), "//", "\r\n");;
        }

        static string Remove(string fileString, string startString, string endString)
        {
            string result = fileString;
            int commentStartIndex = result.IndexOf(startString, StringComparison.Ordinal);
            while (commentStartIndex != -1)
            {
                int commentEndIndex = result.IndexOf(endString, commentStartIndex, StringComparison.Ordinal);
                if (commentEndIndex == -1)
                    commentEndIndex = result.Length - 1;
                result = result.Remove(commentStartIndex, commentEndIndex - commentStartIndex);
                commentStartIndex = result.IndexOf(startString, StringComparison.Ordinal);
            }

            return result;
        }

        static string RemoveWhitespace(string input)
        {
            return new string(input.ToCharArray().Where(c => !Char.IsWhiteSpace(c)).ToArray());
        }

        static int FindFirstNotOf(string source, string chars, int offset = 0)
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

        static int FindClosingBracket(string source, int openingBracketPos)
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
    }
}