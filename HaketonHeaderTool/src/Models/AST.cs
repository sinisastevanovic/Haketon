using System.Collections.Generic;

namespace HaketonHeaderTool
{
    public abstract class ASTNode
    {
        public SourcePosition Position { get; set; }
        
        protected ASTNode(SourcePosition position)
        {
            Position = position;
        }
    }
    
    public class FileNode : ASTNode
    {
        public string FileName { get; set; }
        public List<ASTNode> Children { get; set; } = new List<ASTNode>();
        
        public FileNode(string fileName, SourcePosition position) : base(position)
        {
            FileName = fileName;
        }
    }
    
    public class EnumNode : ASTNode
    {
        public string Name { get; set; }
        public List<EnumValueNode> Values { get; set; } = new List<EnumValueNode>();
        public Metadata Metadata { get; set; }
        
        public EnumNode(string name, SourcePosition position) : base(position)
        {
            Name = name;
        }
    }
    
    public class EnumValueNode : ASTNode
    {
        public string Name { get; set; }
        public string Value { get; set; } // Optional assignment value
        
        public EnumValueNode(string name, SourcePosition position) : base(position)
        {
            Name = name;
        }
    }
    
    public class StructNode : ASTNode
    {
        public string Name { get; set; }
        public string BaseClass { get; set; } // For inheritance
        public List<PropertyNode> Properties { get; set; } = new List<PropertyNode>();
        public List<FunctionNode> Functions { get; set; } = new List<FunctionNode>();
        public Metadata Metadata { get; set; }
        public bool IsComponent { get; set; }
        
        public StructNode(string name, SourcePosition position) : base(position)
        {
            Name = name;
        }
    }
    
    public class PropertyNode : ASTNode
    {
        public string Name { get; set; }
        public string Type { get; set; }
        public Metadata Metadata { get; set; }
        public string AccessModifier { get; set; } = "public"; // public, private, protected
        
        public PropertyNode(string name, string type, SourcePosition position) : base(position)
        {
            Name = name;
            Type = type;
        }
    }
    
    public class FunctionNode : ASTNode
    {
        public string Name { get; set; }
        public string ReturnType { get; set; }
        public List<ParameterNode> Parameters { get; set; } = new List<ParameterNode>();
        public Metadata Metadata { get; set; }
        public string AccessModifier { get; set; } = "public";
        
        public FunctionNode(string name, string returnType, SourcePosition position) : base(position)
        {
            Name = name;
            ReturnType = returnType;
        }
    }
    
    public class ParameterNode : ASTNode
    {
        public string Name { get; set; }
        public string Type { get; set; }
        
        public ParameterNode(string name, string type, SourcePosition position) : base(position)
        {
            Name = name;
            Type = type;
        }
    }
    
    public class ForwardDeclarationNode : ASTNode
    {
        public string Name { get; set; }
        public string Type { get; set; } // "class" or "struct"
        
        public ForwardDeclarationNode(string name, string type, SourcePosition position) : base(position)
        {
            Name = name;
            Type = type;
        }
    }
}