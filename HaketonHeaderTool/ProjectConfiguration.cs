using System.Collections.Generic;

namespace HaketonHeaderTool
{
    public static class ProjectConfiguration
    {
        public static string SolutionDir { get; set; } = "..\\..\\..\\..\\";
        public static string ProjectName { get; set; } = "";
        public static string ProjectSrcDir { get; set; } = "";
        public static string OutputDir { get; set; } = "";

        public static string[] FilesToScan { get; set; }
        public static List<string> GeneratedFunctions { get; set; } = new List<string>();
        public static List<ComponentInfo> DiscoveredComponents { get; set; } = new List<ComponentInfo>();

        public static void Initialize(string solutionDir, string projectName)
        {
            SolutionDir = solutionDir;
            ProjectName = projectName;
            ProjectSrcDir = SolutionDir + ProjectName + "\\src\\";
            OutputDir = ProjectSrcDir + "GeneratedFiles\\";
        }
    }
}