using System;
using System.Collections.Generic;
using System.Linq;

namespace HaketonHeaderTool
{
    public class Parser
    {
        private readonly List<Token> _tokens;
        private int _position;
        private Token CurrentToken => _position < _tokens.Count ? _tokens[_position] : null;
        private Token PeekToken(int offset = 1) => _position + offset < _tokens.Count ? _tokens[_position + offset] : null;
        
        public Parser(List<Token> tokens)
        {
            _tokens = tokens ?? throw new ArgumentNullException(nameof(tokens));
            _position = 0;
        }
        
        public FileNode ParseFile(string fileName)
        {
            var file = new FileNode(fileName, new SourcePosition(fileName));
            
            while (CurrentToken != null && CurrentToken.Type != TokenType.EndOfFile)
            {
                try
                {
                    var node = ParseTopLevelDeclaration();
                    if (node != null)
                    {
                        file.Children.Add(node);
                    }
                }
                catch (ParseException ex)
                {
                    Logger.Error(ex, $"Parse error in {fileName}");
                    // Skip to next token to try to recover
                    AdvanceToken();
                }
                catch (Exception ex)
                {
                    throw new ParseException($"Unexpected parser error: {ex.Message}", fileName, 
                        CurrentToken?.Position.LineNumber ?? -1);
                }
            }
            
            return file;
        }
        
        private ASTNode ParseTopLevelDeclaration()
        {
            SkipWhitespaceAndComments();
            
            if (CurrentToken == null || CurrentToken.Type == TokenType.EndOfFile)
                return null;
            
            switch (CurrentToken.Type)
            {
                case TokenType.Enum:
                    AdvanceToken(); // Consume ENUM token
                    return ParseEnum();
                    
                case TokenType.Struct:
                    AdvanceToken(); // Consume STRUCT token
                    return ParseStruct();
                    
                case TokenType.Class:
                    // Check if this is a forward declaration (class Name;)
                    if (IsForwardDeclaration())
                        return ParseForwardDeclaration();
                    else
                    {
                        // Skip non-reflected classes - only process STRUCT() marked ones
                        SkipToNextDeclaration();
                        return null;
                    }
                        
                default:
                    AdvanceToken();
                    return null;
            }
        }
        
        private EnumNode ParseEnum()
        {
            var startPos = CurrentToken.Position;
            
            // Parse metadata if present
            var metadata = ParseMetadata();
            
            // The ENUM token was already consumed by ParseTopLevelDeclaration
            // Now find the actual enum declaration
            if (!FindNextNonWhitespace())
                throw new ParseException("Unexpected end of file after ENUM", startPos.FileName, startPos.LineNumber);
            
            // Parse: enum [class] Name
            bool isEnumClass = false;
            if (CurrentToken.Type == TokenType.Identifier && CurrentToken.Value == "enum")
            {
                AdvanceToken();
                SkipWhitespaceAndComments();
                
                if (CurrentToken.Type == TokenType.Class)
                {
                    isEnumClass = true;
                    AdvanceToken();
                    SkipWhitespaceAndComments();
                }
            }
            
            // Get enum name
            if (CurrentToken?.Type != TokenType.Identifier)
                throw new ParseException("Expected enum name", CurrentToken?.Position.FileName ?? "", 
                    CurrentToken?.Position.LineNumber ?? -1);
            
            string enumName = CurrentToken.Value;
            var enumNode = new EnumNode(enumName, CurrentToken.Position);
            enumNode.Metadata = metadata;
            
            AdvanceToken();
            SkipWhitespaceAndComments();
            
            // Find opening brace
            if (!FindToken(TokenType.OpenBrace))
                throw new ParseException($"Expected '{{' for enum '{enumName}'", enumName, startPos.LineNumber);
            
            AdvanceToken(); // Skip '{'
            
            // Parse enum values
            while (CurrentToken != null && CurrentToken.Type != TokenType.CloseBrace)
            {
                SkipWhitespaceAndComments();
                
                if (CurrentToken?.Type == TokenType.Identifier)
                {
                    var valueNode = new EnumValueNode(CurrentToken.Value, CurrentToken.Position);
                    AdvanceToken();
                    SkipWhitespaceAndComments();
                    
                    // Check for assignment
                    if (CurrentToken?.Type == TokenType.Assignment)
                    {
                        AdvanceToken();
                        SkipWhitespaceAndComments();
                        
                        if (CurrentToken?.Type == TokenType.NumberLiteral || CurrentToken?.Type == TokenType.Identifier)
                        {
                            valueNode.Value = CurrentToken.Value;
                            AdvanceToken();
                        }
                    }
                    
                    enumNode.Values.Add(valueNode);
                    
                    SkipWhitespaceAndComments();
                    
                    // Skip comma if present
                    if (CurrentToken?.Type == TokenType.Comma)
                    {
                        AdvanceToken();
                    }
                }
                else if (CurrentToken?.Type != TokenType.CloseBrace)
                {
                    AdvanceToken(); // Skip unknown tokens
                }
            }
            
            if (CurrentToken?.Type != TokenType.CloseBrace)
                throw new ParseException($"Expected '}}' for enum '{enumName}'", enumName, startPos.LineNumber);
            
            AdvanceToken(); // Skip '}'
            
            return enumNode;
        }
        
        private StructNode ParseStruct()
        {
            var startPos = CurrentToken.Position;
            
            // Parse metadata if present
            var metadata = ParseMetadata();
            
            // The STRUCT token was already consumed by ParseTopLevelDeclaration
            // Now find the actual struct/class declaration
            if (!FindNextNonWhitespace())
                throw new ParseException("Unexpected end of file after STRUCT", startPos.FileName, startPos.LineNumber);
            
            // Parse: struct/class Name [: BaseClass]
            bool isClass = false;
            if (CurrentToken.Type == TokenType.Class && CurrentToken.Value == "struct")
            {
                AdvanceToken();
            }
            else if (CurrentToken.Type == TokenType.Class && CurrentToken.Value == "class")
            {
                isClass = true;
                AdvanceToken();
            }
            
            SkipWhitespaceAndComments();
            
            // Get struct name
            if (CurrentToken?.Type != TokenType.Identifier)
                throw new ParseException("Expected struct/class name", CurrentToken?.Position.FileName ?? "", 
                    CurrentToken?.Position.LineNumber ?? -1);
            
            string structName = CurrentToken.Value;
            var structNode = new StructNode(structName, CurrentToken.Position);
            structNode.Metadata = metadata;
            
            AdvanceToken();
            SkipWhitespaceAndComments();
            
            // Check for inheritance
            if (CurrentToken?.Type == TokenType.Colon)
            {
                AdvanceToken();
                SkipWhitespaceAndComments();
                
                if (CurrentToken?.Type == TokenType.Identifier)
                {
                    string baseClass = ParseQualifiedIdentifier();
                    if (baseClass != null)
                    {
                        structNode.BaseClass = baseClass;
                        
                        // Check if it's a Component (could be qualified like Engine::Component)
                        if (baseClass == "Component" || baseClass.EndsWith("::Component"))
                        {
                            structNode.IsComponent = true;
                        }
                    }
                    
                    SkipWhitespaceAndComments();
                }
            }
            
            // Find opening brace
            if (!FindToken(TokenType.OpenBrace))
                throw new ParseException($"Expected '{{' for struct '{structName}'", structName, startPos.LineNumber);
            
            AdvanceToken(); // Skip '{'
            
            // Parse struct body
            while (CurrentToken != null && CurrentToken.Type != TokenType.CloseBrace)
            {
                SkipWhitespaceAndComments();
                
                if (CurrentToken?.Type == TokenType.CloseBrace)
                    break;
                
                // Parse properties and functions
                var member = ParseStructMember();
                if (member is PropertyNode property)
                {
                    structNode.Properties.Add(property);
                }
                else if (member is FunctionNode function)
                {
                    structNode.Functions.Add(function);
                }
                
                // Also check for RTTR_ENABLE
                if (CurrentToken?.Type == TokenType.Identifier && CurrentToken.Value == "RTTR_ENABLE")
                {
                    structNode.IsComponent = true;
                    AdvanceToken();
                }
            }
            
            if (CurrentToken?.Type != TokenType.CloseBrace)
                throw new ParseException($"Expected '}}' for struct '{structName}'", structName, startPos.LineNumber);
            
            AdvanceToken(); // Skip '}'
            
            return structNode;
        }
        
        private ASTNode ParseStructMember()
        {
            SkipWhitespaceAndComments();
            
            if (CurrentToken == null)
                return null;
            
            
            switch (CurrentToken.Type)
            {
                case TokenType.Property:
                    AdvanceToken(); // Consume PROPERTY token
                    return ParseProperty();
                    
                case TokenType.Function:
                    AdvanceToken(); // Consume FUNCTION token
                    return ParseFunction();
                    
                default:
                    // Skip unknown tokens until we find something we recognize or reach end of struct
                    while (CurrentToken != null && 
                           CurrentToken.Type != TokenType.CloseBrace &&
                           CurrentToken.Type != TokenType.Property &&
                           CurrentToken.Type != TokenType.Function)
                    {
                        // Handle brace-enclosed expressions (like initializations)
                        if (CurrentToken.Type == TokenType.OpenBrace)
                        {
                            SkipBraceEnclosedExpression();
                        }
                        else
                        {
                            AdvanceToken();
                        }
                    }
                    return null;
            }
        }
        
        private PropertyNode ParseProperty()
        {
            var startPos = CurrentToken.Position;
            
            // Parse metadata
            var metadata = ParseMetadata();
            
            // The PROPERTY token was already consumed by ParseStructMember
            // Find the property declaration
            if (!FindNextNonWhitespace())
                throw new ParseException("Unexpected end of file after PROPERTY", startPos.FileName, startPos.LineNumber);
            
            // Parse: Type propertyName;
            if (CurrentToken?.Type != TokenType.Identifier)
                throw new ParseException("Expected property type", CurrentToken?.Position.FileName ?? "", 
                    CurrentToken?.Position.LineNumber ?? -1);
            
            string propertyType = ParseQualifiedIdentifier();
            if (propertyType == null)
                throw new ParseException("Expected property type", CurrentToken?.Position.FileName ?? "", 
                    CurrentToken?.Position.LineNumber ?? -1);
            
            SkipWhitespaceAndComments();
            
            if (CurrentToken?.Type != TokenType.Identifier)
                throw new ParseException("Expected property name", CurrentToken?.Position.FileName ?? "", 
                    CurrentToken?.Position.LineNumber ?? -1);
            
            string propertyName = CurrentToken.Value;
            var propertyNode = new PropertyNode(propertyName, propertyType, CurrentToken.Position);
            propertyNode.Metadata = metadata;
            
            
            AdvanceToken();
            
            return propertyNode;
        }
        
        private FunctionNode ParseFunction()
        {
            var startPos = CurrentToken.Position;
            
            // Parse metadata
            var metadata = ParseMetadata();
            
            // The FUNCTION token was already consumed by ParseStructMember
            // Find the function declaration
            if (!FindNextNonWhitespace())
                throw new ParseException("Unexpected end of file after FUNCTION", startPos.FileName, startPos.LineNumber);
            
            // Parse: ReturnType functionName(params);
            if (CurrentToken?.Type != TokenType.Identifier)
                throw new ParseException("Expected function return type", CurrentToken?.Position.FileName ?? "", 
                    CurrentToken?.Position.LineNumber ?? -1);
            
            string returnType = ParseQualifiedIdentifier();
            if (returnType == null)
                throw new ParseException("Expected function return type", CurrentToken?.Position.FileName ?? "", 
                    CurrentToken?.Position.LineNumber ?? -1);
            
            SkipWhitespaceAndComments();
            
            if (CurrentToken?.Type != TokenType.Identifier)
                throw new ParseException("Expected function name", CurrentToken?.Position.FileName ?? "", 
                    CurrentToken?.Position.LineNumber ?? -1);
            
            string functionName = CurrentToken.Value;
            var functionNode = new FunctionNode(functionName, returnType, CurrentToken.Position);
            functionNode.Metadata = metadata;
            
            AdvanceToken();
            
            return functionNode;
        }
        
        private ForwardDeclarationNode ParseForwardDeclaration()
        {
            var startPos = CurrentToken.Position;
            string type = CurrentToken.Value; // "class" or "struct"
            
            AdvanceToken();
            SkipWhitespaceAndComments();
            
            if (CurrentToken?.Type != TokenType.Identifier)
                throw new ParseException("Expected class/struct name in forward declaration", 
                    CurrentToken?.Position.FileName ?? "", CurrentToken?.Position.LineNumber ?? -1);
            
            string name = CurrentToken.Value;
            AdvanceToken();
            
            return new ForwardDeclarationNode(name, type, startPos);
        }
        
        private Metadata ParseMetadata()
        {
            var metadata = new Metadata();
            
            // Look for opening parenthesis at current position (token already consumed)
            if (CurrentToken?.Type == TokenType.OpenParen)
            {
                metadata.Position = CurrentToken.Position;
                AdvanceToken(); // Skip '('
                
                // Parse metadata key-value pairs
                while (CurrentToken != null && CurrentToken.Type != TokenType.CloseParen)
                {
                    SkipWhitespace();
                    
                    if (CurrentToken?.Type == TokenType.Identifier)
                    {
                        string key = CurrentToken.Value;
                        AdvanceToken();
                        SkipWhitespace();
                        
                        if (CurrentToken?.Type == TokenType.Assignment)
                        {
                            AdvanceToken();
                            SkipWhitespace();
                            
                            if (CurrentToken?.Type == TokenType.StringLiteral)
                            {
                                // Remove quotes from string literal
                                string value = CurrentToken.Value;
                                if (value.Length >= 2 && value.StartsWith("\"") && value.EndsWith("\""))
                                {
                                    value = value.Substring(1, value.Length - 2);
                                }
                                metadata.Properties[key] = value;
                                AdvanceToken();
                            }
                            else if (CurrentToken?.Type == TokenType.Identifier || CurrentToken?.Type == TokenType.NumberLiteral)
                            {
                                metadata.Properties[key] = CurrentToken.Value;
                                AdvanceToken();
                            }
                        }
                        else
                        {
                            // Flag without value
                            metadata.Properties[key] = "true";
                        }
                        
                        SkipWhitespace();
                        
                        // Skip comma
                        if (CurrentToken?.Type == TokenType.Comma)
                        {
                            AdvanceToken();
                        }
                    }
                    else
                    {
                        AdvanceToken();
                    }
                }
                
                if (CurrentToken?.Type == TokenType.CloseParen)
                {
                    AdvanceToken();
                }
                
                return metadata;
            }
            
            return metadata;
        }
        
        private bool IsForwardDeclaration()
        {
            // Look ahead to see if this is a forward declaration (ends with semicolon before any braces)
            int lookahead = _position;
            while (lookahead < _tokens.Count)
            {
                var token = _tokens[lookahead];
                if (token.Type == TokenType.OpenBrace)
                    return false;
                if (token.Type == TokenType.Semicolon)
                    return true;
                if (token.Type == TokenType.EndOfFile || token.Type == TokenType.CloseBrace)
                    return false;
                lookahead++;
            }
            return false;
        }
        
        private bool FindToken(TokenType type)
        {
            while (CurrentToken != null && CurrentToken.Type != TokenType.EndOfFile)
            {
                if (CurrentToken.Type == type)
                    return true;
                AdvanceToken();
            }
            return false;
        }
        
        private bool FindNextNonWhitespace()
        {
            while (CurrentToken != null && 
                   (CurrentToken.Type == TokenType.Whitespace || 
                    CurrentToken.Type == TokenType.Newline ||
                    CurrentToken.Type == TokenType.Comment))
            {
                AdvanceToken();
            }
            return CurrentToken != null && CurrentToken.Type != TokenType.EndOfFile;
        }
        
        private void SkipWhitespaceAndComments()
        {
            while (CurrentToken != null && 
                   (CurrentToken.Type == TokenType.Whitespace || 
                    CurrentToken.Type == TokenType.Newline || 
                    CurrentToken.Type == TokenType.Comment))
            {
                AdvanceToken();
            }
        }
        
        private void SkipWhitespace()
        {
            while (CurrentToken != null && CurrentToken.Type == TokenType.Whitespace)
            {
                AdvanceToken();
            }
        }
        
        private void SkipToNextDeclaration()
        {
            // Skip until we find the end of this class/struct declaration
            int braceLevel = 0;
            bool foundOpenBrace = false;
            
            while (CurrentToken != null && CurrentToken.Type != TokenType.EndOfFile)
            {
                if (CurrentToken.Type == TokenType.OpenBrace)
                {
                    foundOpenBrace = true;
                    braceLevel++;
                }
                else if (CurrentToken.Type == TokenType.CloseBrace)
                {
                    braceLevel--;
                    if (foundOpenBrace && braceLevel == 0)
                    {
                        AdvanceToken(); // Skip the closing brace
                        break;
                    }
                }
                else if (CurrentToken.Type == TokenType.Semicolon && !foundOpenBrace)
                {
                    // This was just a forward declaration, skip the semicolon
                    AdvanceToken();
                    break;
                }
                
                AdvanceToken();
            }
        }
        
        private void AdvanceToken()
        {
            if (_position < _tokens.Count)
                _position++;
        }
        
        private void SkipBraceEnclosedExpression()
        {
            if (CurrentToken?.Type != TokenType.OpenBrace)
                return;
                
            int braceDepth = 1;
            AdvanceToken(); // Skip opening brace
            
            while (CurrentToken != null && braceDepth > 0)
            {
                if (CurrentToken.Type == TokenType.OpenBrace)
                {
                    braceDepth++;
                }
                else if (CurrentToken.Type == TokenType.CloseBrace)
                {
                    braceDepth--;
                }
                
                AdvanceToken();
            }
        }
        
        private string ParseQualifiedIdentifier()
        {
            if (CurrentToken?.Type != TokenType.Identifier)
                return null;
                
            var sb = new System.Text.StringBuilder();
            sb.Append(CurrentToken.Value);
            AdvanceToken();
            
            // Parse template parameters if present: Identifier<TemplateArgs>
            if (CurrentToken?.Type == TokenType.OpenAngle)
            {
                sb.Append(ParseTemplateArguments());
            }
            
            // Parse namespace qualifiers: identifier::identifier::identifier
            while (CurrentToken?.Type == TokenType.ScopeResolution)
            {
                sb.Append("::");
                AdvanceToken();
                
                if (CurrentToken?.Type == TokenType.Identifier)
                {
                    sb.Append(CurrentToken.Value);
                    AdvanceToken();
                    
                    // Check for template parameters on this part too
                    if (CurrentToken?.Type == TokenType.OpenAngle)
                    {
                        sb.Append(ParseTemplateArguments());
                    }
                }
                else
                {
                    throw new ParseException("Expected identifier after '::'", 
                        CurrentToken?.Position.FileName ?? "", 
                        CurrentToken?.Position.LineNumber ?? -1);
                }
            }
            
            return sb.ToString();
        }
        
        private string ParseTemplateArguments()
        {
            if (CurrentToken?.Type != TokenType.OpenAngle)
                return "";
                
            var sb = new System.Text.StringBuilder();
            sb.Append("<");
            AdvanceToken(); // Skip '<'
            
            int angleDepth = 1;
            
            // Parse everything inside angle brackets, handling nested templates
            while (CurrentToken != null && angleDepth > 0)
            {
                switch (CurrentToken.Type)
                {
                    case TokenType.OpenAngle:
                        angleDepth++;
                        sb.Append("<");
                        break;
                        
                    case TokenType.CloseAngle:
                        angleDepth--;
                        sb.Append(">");
                        break;
                        
                    case TokenType.ScopeResolution:
                        sb.Append("::");
                        break;
                        
                    case TokenType.Comma:
                        sb.Append(", ");
                        break;
                        
                    case TokenType.Whitespace:
                        // Skip whitespace but add a single space if needed
                        if (sb.Length > 0 && sb[sb.Length - 1] != ' ' && sb[sb.Length - 1] != '<')
                            sb.Append(" ");
                        break;
                        
                    default:
                        // Add the token value (identifiers, numbers, etc.)
                        sb.Append(CurrentToken.Value);
                        break;
                }
                
                AdvanceToken();
            }
            
            return sb.ToString();
        }
    }
}