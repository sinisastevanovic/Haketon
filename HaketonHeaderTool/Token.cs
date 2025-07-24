using System.Collections.Generic;

namespace HaketonHeaderTool
{
    public enum TokenType
    {
        // Reflection tokens
        Struct,
        Enum,
        Property,
        Function,
        
        // C++ keywords
        Class,
        Namespace,
        Public,
        Private,
        Protected,
        
        // Symbols
        OpenBrace,      // {
        CloseBrace,     // }
        OpenParen,      // (
        CloseParen,     // )
        OpenAngle,      // <
        CloseAngle,     // >
        Semicolon,      // ;
        Comma,          // ,
        Assignment,     // =
        Colon,          // :
        ScopeResolution, // ::
        
        // Literals
        Identifier,
        StringLiteral,
        NumberLiteral,
        
        // Special
        Newline,
        Whitespace,
        Comment,
        EndOfFile,
        Unknown
    }
    
    public class Token
    {
        public TokenType Type { get; set; }
        public string Value { get; set; }
        public SourcePosition Position { get; set; }
        
        public Token(TokenType type, string value, SourcePosition position)
        {
            Type = type;
            Value = value;
            Position = position;
        }
        
        public override string ToString()
        {
            return $"{Type}: '{Value}' at {Position}";
        }
    }
    
    public class Metadata
    {
        public Dictionary<string, string> Properties { get; set; } = new Dictionary<string, string>();
        public SourcePosition Position { get; set; }
        
        public string GetProperty(string key, string defaultValue = "")
        {
            return Properties.TryGetValue(key, out string value) ? value : defaultValue;
        }
        
        public bool HasProperty(string key)
        {
            return Properties.ContainsKey(key);
        }
        
        public bool GetBoolProperty(string key, bool defaultValue = false)
        {
            if (Properties.TryGetValue(key, out string value))
            {
                if (bool.TryParse(value, out bool result))
                    return result;
                // If it's just a flag without value, consider it true
                return true;
            }
            return defaultValue;
        }
    }
}