using System;
using System.IO;

namespace HaketonHeaderTool
{
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
}