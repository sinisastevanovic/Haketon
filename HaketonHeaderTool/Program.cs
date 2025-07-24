using System;
using System.IO;

namespace HaketonHeaderTool
{
    public class Program
    {
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
            // Initialize project configuration
            ProjectConfiguration.Initialize(solutionDir, projectName);

            // Initialize file processor
            var fileProcessor = new FileProcessor();
            
            // Discover files to scan
            ProjectConfiguration.FilesToScan = fileProcessor.DiscoverFilesToScan(ProjectConfiguration.ProjectSrcDir);
            if (ProjectConfiguration.FilesToScan.Length == 0)
            {
                Logger.Warning($"No header files found in project '{projectName}'. Skipping code generation.");
                return;
            }
            
            // Clean up previously generated files
            fileProcessor.CleanupPreviouslyGeneratedFiles(ProjectConfiguration.OutputDir);
            
            // Process all files
            var results = fileProcessor.ProcessFiles(ProjectConfiguration.FilesToScan, ProjectConfiguration.ProjectSrcDir);
            
            // Generate master registration header if we have generated functions
            if (ProjectConfiguration.GeneratedFunctions.Count > 0)
            {
                GenerateMasterRegistrationHeader();
            }
                
            // Generate component registry files if we have discovered components
            if (ProjectConfiguration.DiscoveredComponents.Count > 0)
            {
                GenerateComponentRegistryFiles(projectName);
            }
        }
        
        static void GenerateMasterRegistrationHeader()
        {
            string masterHeaderContent = "#pragma once\n\nnamespace Haketon\n{\n";
            
            // Declare all registration functions
            foreach (string functionName in ProjectConfiguration.GeneratedFunctions)
            {
                masterHeaderContent += "\tvoid " + functionName + "();\n";
            }
            
            // Create master registration function
            masterHeaderContent += "\n\tinline void RegisterAll" + ProjectConfiguration.ProjectName + "Types()\n\t{\n";
            foreach (string functionName in ProjectConfiguration.GeneratedFunctions)
            {
                masterHeaderContent += "\t\t" + functionName + "();\n";
            }
            masterHeaderContent += "\t}\n}\n";
            
            string masterHeaderPath = ProjectConfiguration.OutputDir + "AutoReflection.gen.h";
            File.WriteAllText(masterHeaderPath, masterHeaderContent);
            Console.WriteLine($"Generated master registration header: {masterHeaderPath}");
        }
        
        static void GenerateComponentRegistryFiles(string projectName)
        {
            if (projectName == "Haketon")
            {
                // Generate component registry entries for HaketonEditor to include
                ComponentRegistryGenerator.GenerateHaketonComponentRegistry(
                    ProjectConfiguration.DiscoveredComponents, 
                    ProjectConfiguration.OutputDir);
            }
            else if (projectName == "HaketonEditor")
            {
                // Generate complete component registry for HaketonEditor
                ComponentRegistryGenerator.GenerateComponentRegistry(
                    ProjectConfiguration.DiscoveredComponents, 
                    ProjectConfiguration.SolutionDir, 
                    ProjectConfiguration.OutputDir);
            }
        }
    }
}