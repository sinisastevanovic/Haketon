using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace HaketonHeaderTool
{
    public class FileProcessor
    {
        private readonly List<string> _filesToIgnore = new List<string> { @"Haketon\Core\Core.h", @"hkpch.h", @"Haketon.h" };
        
        public string[] DiscoverFilesToScan(string projectSrcDir)
        {
            if (!Directory.Exists(projectSrcDir))
            {
                throw new FileProcessingException($"Project source directory '{projectSrcDir}' does not exist!");
            }

            Logger.Info($"Scanning files in directory '{projectSrcDir}'...");

            var filesToScan = Directory.GetFiles(projectSrcDir, "*.h", SearchOption.AllDirectories)
                .Where(file => !file.ToLower().EndsWith(".gen.h")).ToArray();
                
            if (filesToScan.Length == 0)
            {
                Logger.Warning($"No header files found in directory '{projectSrcDir}'. Skipping code generation.");
                return new string[0];
            }
            
            Logger.Info($"Found {filesToScan.Length} header files to process");
            return filesToScan;
        }
        
        public void CleanupPreviouslyGeneratedFiles(string outputDir)
        {
            if (Directory.Exists(outputDir))
            {
                string[] existingFiles = Directory.GetFiles(outputDir, "*.*", SearchOption.AllDirectories)
                    .Where(file => file.ToLower().EndsWith(".gen.cpp") || file.ToLower().EndsWith(".gen.h")).ToArray();
                    
                foreach (string existingFile in existingFiles)
                {
                    _filesToIgnore.Add(existingFile);
                    File.Delete(existingFile);
                }
            }
        }
        
        public ProcessingResults ProcessFiles(string[] filesToScan, string dirToSearch)
        {
            int processedCount = 0;
            int errorCount = 0;
            
            foreach (string file in filesToScan)
            {
                string currentRelativeDir = file.Replace(dirToSearch, "");
                string fileName = Path.GetFileNameWithoutExtension(currentRelativeDir);
                string includeDir = currentRelativeDir.Replace(fileName + ".h", "");

                if (!_filesToIgnore.Contains(currentRelativeDir))
                {
                    try
                    {
                        var headerFileInfo = new HeaderFileInfo(dirToSearch, includeDir, fileName);
                        Logger.Debug($"Processing header: {currentRelativeDir}");
                        
                        if (HeaderParser.GenerateHeaderForHeader(headerFileInfo))
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
            
            return new ProcessingResults
            {
                ProcessedCount = processedCount,
                ErrorCount = errorCount
            };
        }
    }
    
    public class ProcessingResults
    {
        public int ProcessedCount { get; set; }
        public int ErrorCount { get; set; }
    }
}