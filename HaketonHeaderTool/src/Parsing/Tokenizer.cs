using System;
using System.Collections.Generic;
using System.Text;

namespace HaketonHeaderTool
{
    public class Tokenizer
    {
        private readonly string _source;
        private readonly SourceTracker _sourceTracker;
        private int _position;
        private bool _foundRelevantToken;

        public bool FoundRelevantToken => _foundRelevantToken;

        private static readonly Dictionary<string, TokenType> Keywords = new Dictionary<string, TokenType>
        {
            { "STRUCT", TokenType.Struct },
            { "ENUM", TokenType.Enum },
            { "PROPERTY", TokenType.Property },
            { "FUNCTION", TokenType.Function },
            { "class", TokenType.Class },
            { "struct", TokenType.Class }, // Treat struct as class for parsing
            { "namespace", TokenType.Namespace },
            { "public", TokenType.Public },
            { "private", TokenType.Private },
            { "protected", TokenType.Protected }
        };
        
        public Tokenizer(string source, string fileName)
        {
            _source = source ?? throw new ArgumentNullException(nameof(source));
            _sourceTracker = new SourceTracker(source, fileName);
            _position = 0;
            _foundRelevantToken = false;
        }
        
        public List<Token> Tokenize()
        {
            var tokens = new List<Token>();
            
            while (_position < _source.Length)
            {
                var token = NextToken();
                if (token != null)
                {
                    tokens.Add(token);
                    
                    if (token.Type == TokenType.EndOfFile)
                        break;
                }
            }
            
            return tokens;
        }
        
        private Token NextToken()
        {
            if (_position >= _source.Length)
                return new Token(TokenType.EndOfFile, "", _sourceTracker.GetPositionAt(_position));
            
            var startPos = _sourceTracker.GetPositionAt(_position);
            char current = _source[_position];
            
            // Skip whitespace (but track newlines)
            if (char.IsWhiteSpace(current))
            {
                if (current == '\r' || current == '\n')
                {
                    return ConsumeNewline();
                }
                else
                {
                    return ConsumeWhitespace();
                }
            }
            
            // String literals
            if (current == '"' || current == '\'')
            {
                return ConsumeStringLiteral();
            }
            
            // Comments
            if (current == '/' && _position + 1 < _source.Length)
            {
                char next = _source[_position + 1];
                if (next == '/' || next == '*')
                {
                    return ConsumeComment();
                }
            }
            
            // Multi-character tokens
            if (current == ':' && _position + 1 < _source.Length && _source[_position + 1] == ':')
            {
                _position += 2;
                return new Token(TokenType.ScopeResolution, "::", startPos);
            }
            
            // Single character tokens
            switch (current)
            {
                case '{': _position++; return new Token(TokenType.OpenBrace, "{", startPos);
                case '}': _position++; return new Token(TokenType.CloseBrace, "}", startPos);
                case '(': _position++; return new Token(TokenType.OpenParen, "(", startPos);
                case ')': _position++; return new Token(TokenType.CloseParen, ")", startPos);
                case '<': _position++; return new Token(TokenType.OpenAngle, "<", startPos);
                case '>': _position++; return new Token(TokenType.CloseAngle, ">", startPos);
                case ';': _position++; return new Token(TokenType.Semicolon, ";", startPos);
                case ',': _position++; return new Token(TokenType.Comma, ",", startPos);
                case '=': _position++; return new Token(TokenType.Assignment, "=", startPos);
                case ':': _position++; return new Token(TokenType.Colon, ":", startPos);
            }
            
            // Numbers
            if (char.IsDigit(current))
            {
                return ConsumeNumber();
            }
            
            // Identifiers and keywords
            if (char.IsLetter(current) || current == '_')
            {
                return ConsumeIdentifier();
            }
            
            // Unknown character - consume it and continue
            _position++;
            return new Token(TokenType.Unknown, current.ToString(), startPos);
        }
        
        private Token ConsumeNewline()
        {
            var startPos = _sourceTracker.GetPositionAt(_position);
            var sb = new StringBuilder();
            
            while (_position < _source.Length)
            {
                char c = _source[_position];
                if (c == '\r' || c == '\n')
                {
                    sb.Append(c);
                    _position++;
                }
                else
                {
                    break;
                }
            }
            
            return new Token(TokenType.Newline, sb.ToString(), startPos);
        }
        
        private Token ConsumeWhitespace()
        {
            var startPos = _sourceTracker.GetPositionAt(_position);
            var sb = new StringBuilder();
            
            while (_position < _source.Length)
            {
                char c = _source[_position];
                if (char.IsWhiteSpace(c) && c != '\r' && c != '\n')
                {
                    sb.Append(c);
                    _position++;
                }
                else
                {
                    break;
                }
            }
            
            return new Token(TokenType.Whitespace, sb.ToString(), startPos);
        }
        
        private Token ConsumeStringLiteral()
        {
            var startPos = _sourceTracker.GetPositionAt(_position);
            char quote = _source[_position];
            var sb = new StringBuilder();
            sb.Append(quote);
            _position++;
            
            while (_position < _source.Length)
            {
                char c = _source[_position];
                sb.Append(c);
                
                if (c == '\\' && _position + 1 < _source.Length)
                {
                    // Escape sequence
                    _position++;
                    sb.Append(_source[_position]);
                    _position++;
                }
                else if (c == quote)
                {
                    // End of string
                    _position++;
                    break;
                }
                else
                {
                    _position++;
                }
            }
            
            return new Token(TokenType.StringLiteral, sb.ToString(), startPos);
        }
        
        private Token ConsumeComment()
        {
            var startPos = _sourceTracker.GetPositionAt(_position);
            var sb = new StringBuilder();
            
            if (_source[_position + 1] == '/')
            {
                // Single line comment
                while (_position < _source.Length && _source[_position] != '\r' && _source[_position] != '\n')
                {
                    sb.Append(_source[_position]);
                    _position++;
                }
            }
            else if (_source[_position + 1] == '*')
            {
                // Multi-line comment
                _position += 2;
                sb.Append("/*");
                
                while (_position + 1 < _source.Length)
                {
                    if (_source[_position] == '*' && _source[_position + 1] == '/')
                    {
                        sb.Append("*/");
                        _position += 2;
                        break;
                    }
                    sb.Append(_source[_position]);
                    _position++;
                }
            }
            
            return new Token(TokenType.Comment, sb.ToString(), startPos);
        }
        
        private Token ConsumeNumber()
        {
            var startPos = _sourceTracker.GetPositionAt(_position);
            var sb = new StringBuilder();
            
            while (_position < _source.Length)
            {
                char c = _source[_position];
                if (char.IsDigit(c) || c == '.' || c == 'f' || c == 'F' || c == 'l' || c == 'L')
                {
                    sb.Append(c);
                    _position++;
                }
                else
                {
                    break;
                }
            }
            
            return new Token(TokenType.NumberLiteral, sb.ToString(), startPos);
        }
        
        private Token ConsumeIdentifier()
        {
            var startPos = _sourceTracker.GetPositionAt(_position);
            var sb = new StringBuilder();
            
            while (_position < _source.Length)
            {
                char c = _source[_position];
                if (char.IsLetterOrDigit(c) || c == '_')
                {
                    sb.Append(c);
                    _position++;
                }
                else
                {
                    break;
                }
            }
            
            string identifier = sb.ToString();
            TokenType type = Keywords.TryGetValue(identifier, out TokenType keywordType) 
                ? keywordType 
                : TokenType.Identifier;

            if (type == TokenType.Struct || type == TokenType.Enum || type == TokenType.Property ||
                type == TokenType.Function)
                _foundRelevantToken = true;
            
            return new Token(type, identifier, startPos);
        }
    }
}