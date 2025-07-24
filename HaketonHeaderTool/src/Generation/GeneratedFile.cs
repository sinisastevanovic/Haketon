using System.Collections.Generic;

namespace HaketonHeaderTool
{
    public class GeneratedFile
    {
        public string FileName { get; set; }
        public string Content { get; set; }
        public List<string> AdditionalIncludes { get; set; } = new List<string>();
        
        public GeneratedFile(string fileName, string content)
        {
            FileName = fileName;
            Content = content;
        }
    }
}