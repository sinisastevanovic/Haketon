using System.Linq;
using FluentAssertions;
using Xunit;

namespace HaketonHeaderTool.Tests
{
    public class TokenizerTests
    {
        [Fact]
        public void Tokenize_EmptyString_ReturnsEmptyList()
        {
            var tokenizer = new Tokenizer("", "test.h");
            var tokens = tokenizer.Tokenize();
            
            tokens.Should().BeEmpty();
        }

        [Fact]
        public void Tokenize_SingleKeyword_ReturnsCorrectToken()
        {
            var tokenizer = new Tokenizer("STRUCT", "test.h");
            var tokens = tokenizer.Tokenize();
            
            tokens.Should().HaveCount(1);
            tokens[0].Type.Should().Be(TokenType.Struct);
            tokens[0].Value.Should().Be("STRUCT");
            tokenizer.FoundRelevantToken.Should().BeTrue();
        }

        [Theory]
        [InlineData("STRUCT", TokenType.Struct)]
        [InlineData("ENUM", TokenType.Enum)]
        [InlineData("PROPERTY", TokenType.Property)]
        [InlineData("FUNCTION", TokenType.Function)]
        [InlineData("class", TokenType.Class)]
        [InlineData("struct", TokenType.Class)]
        [InlineData("namespace", TokenType.Namespace)]
        [InlineData("public", TokenType.Public)]
        [InlineData("private", TokenType.Private)]
        [InlineData("protected", TokenType.Protected)]
        public void Tokenize_Keywords_ReturnsCorrectTokenTypes(string keyword, TokenType expectedType)
        {
            var tokenizer = new Tokenizer(keyword, "test.h");
            var tokens = tokenizer.Tokenize();
            
            tokens.Should().HaveCount(1);
            tokens[0].Type.Should().Be(expectedType);
            tokens[0].Value.Should().Be(keyword);
        }

        [Theory]
        [InlineData("{", TokenType.OpenBrace)]
        [InlineData("}", TokenType.CloseBrace)]
        [InlineData("(", TokenType.OpenParen)]
        [InlineData(")", TokenType.CloseParen)]
        [InlineData("<", TokenType.OpenAngle)]
        [InlineData(">", TokenType.CloseAngle)]
        [InlineData(";", TokenType.Semicolon)]
        [InlineData(",", TokenType.Comma)]
        [InlineData("=", TokenType.Assignment)]
        [InlineData(":", TokenType.Colon)]
        [InlineData("::", TokenType.ScopeResolution)]
        public void Tokenize_Symbols_ReturnsCorrectTokenTypes(string symbol, TokenType expectedType)
        {
            var tokenizer = new Tokenizer(symbol, "test.h");
            var tokens = tokenizer.Tokenize();
            
            tokens.Should().HaveCount(1);
            tokens[0].Type.Should().Be(expectedType);
            tokens[0].Value.Should().Be(symbol);
        }

        [Fact]
        public void Tokenize_Identifier_ReturnsIdentifierToken()
        {
            var tokenizer = new Tokenizer("myVariable", "test.h");
            var tokens = tokenizer.Tokenize();
            
            tokens.Should().HaveCount(1);
            tokens[0].Type.Should().Be(TokenType.Identifier);
            tokens[0].Value.Should().Be("myVariable");
        }

        [Theory]
        [InlineData("123", "123")]
        [InlineData("456.789", "456.789")]
        [InlineData("123.45f", "123.45f")]
        [InlineData("789L", "789L")]
        public void Tokenize_Numbers_ReturnsNumberLiteralTokens(string number, string expectedValue)
        {
            var tokenizer = new Tokenizer(number, "test.h");
            var tokens = tokenizer.Tokenize();
            
            tokens.Should().HaveCount(1);
            tokens[0].Type.Should().Be(TokenType.NumberLiteral);
            tokens[0].Value.Should().Be(expectedValue);
        }

        [Theory]
        [InlineData("\"hello world\"", "\"hello world\"")]
        [InlineData("'single quote'", "'single quote'")]
        [InlineData("\"escaped \\\"quotes\\\"\"", "\"escaped \\\"quotes\\\"\"")]
        [InlineData("\"multiline\\nstring\"", "\"multiline\\nstring\"")]
        public void Tokenize_StringLiterals_ReturnsStringLiteralTokens(string input, string expectedValue)
        {
            var tokenizer = new Tokenizer(input, "test.h");
            var tokens = tokenizer.Tokenize();
            
            tokens.Should().HaveCount(1);
            tokens[0].Type.Should().Be(TokenType.StringLiteral);
            tokens[0].Value.Should().Be(expectedValue);
        }

        [Theory]
        [InlineData("// single line comment", "// single line comment")]
        [InlineData("/* multi line comment */", "/* multi line comment */")]
        [InlineData("/* multi\nline\ncomment */", "/* multi\nline\ncomment */")]
        public void Tokenize_Comments_ReturnsCommentTokens(string input, string expectedValue)
        {
            var tokenizer = new Tokenizer(input, "test.h");
            var tokens = tokenizer.Tokenize();
            
            tokens.Should().HaveCount(1);
            tokens[0].Type.Should().Be(TokenType.Comment);
            tokens[0].Value.Should().Be(expectedValue);
        }

        [Fact]
        public void Tokenize_Whitespace_ReturnsWhitespaceToken()
        {
            var tokenizer = new Tokenizer("   \t  ", "test.h");
            var tokens = tokenizer.Tokenize();
            
            tokens.Should().HaveCount(1);
            tokens[0].Type.Should().Be(TokenType.Whitespace);
            tokens[0].Value.Should().Be("   \t  ");
        }

        [Fact]
        public void Tokenize_Newlines_ReturnsNewlineTokens()
        {
            var tokenizer = new Tokenizer("\n\r\n", "test.h");
            var tokens = tokenizer.Tokenize();
            
            tokens.Should().HaveCount(2);
            tokens[0].Type.Should().Be(TokenType.Newline);
            tokens[1].Type.Should().Be(TokenType.Newline);
        }

        [Fact]
        public void Tokenize_ComplexCppClass_ReturnsCorrectTokenSequence()
        {
            var source = @"STRUCT()
class MyClass
{
public:
    PROPERTY()
    int value = 42;
    
    FUNCTION()
    void DoSomething();
};";
            
            var tokenizer = new Tokenizer(source, "test.h");
            var tokens = tokenizer.Tokenize();
            
            var relevantTokens = tokens.Where(t => t.Type != TokenType.Whitespace && t.Type != TokenType.Newline).ToList();
            
            relevantTokens[0].Type.Should().Be(TokenType.Struct);
            relevantTokens[1].Type.Should().Be(TokenType.OpenParen);
            relevantTokens[2].Type.Should().Be(TokenType.CloseParen);
            relevantTokens[3].Type.Should().Be(TokenType.Class);
            relevantTokens[4].Type.Should().Be(TokenType.Identifier);
            relevantTokens[4].Value.Should().Be("MyClass");
            relevantTokens[5].Type.Should().Be(TokenType.OpenBrace);
            relevantTokens[6].Type.Should().Be(TokenType.Public);
            relevantTokens[7].Type.Should().Be(TokenType.Colon);
            
            tokenizer.FoundRelevantToken.Should().BeTrue();
        }

        [Fact]
        public void Tokenize_SourcePositionTracking_ReturnsCorrectPositions()
        {
            var source = "STRUCT\nclass Test";
            var tokenizer = new Tokenizer(source, "test.h");
            var tokens = tokenizer.Tokenize();
            
            var relevantTokens = tokens.Where(t => t.Type != TokenType.Whitespace && t.Type != TokenType.Newline).ToList();
            
            relevantTokens[0].Position.LineNumber.Should().Be(1);
            relevantTokens[0].Position.Column.Should().Be(1);
            relevantTokens[1].Position.LineNumber.Should().Be(2);
            relevantTokens[1].Position.Column.Should().Be(1);
            relevantTokens[2].Position.LineNumber.Should().Be(2);
            relevantTokens[2].Position.Column.Should().Be(7);
        }

        [Fact]
        public void Tokenize_UnknownCharacter_ReturnsUnknownToken()
        {
            var tokenizer = new Tokenizer("@", "test.h");
            var tokens = tokenizer.Tokenize();
            
            tokens.Should().HaveCount(1);
            tokens[0].Type.Should().Be(TokenType.Unknown);
            tokens[0].Value.Should().Be("@");
        }

        [Fact]
        public void FoundRelevantToken_WithReflectionTokens_ReturnsTrue()
        {
            var tokenizer = new Tokenizer("PROPERTY", "test.h");
            tokenizer.Tokenize();
            
            tokenizer.FoundRelevantToken.Should().BeTrue();
        }

        [Fact]
        public void FoundRelevantToken_WithoutReflectionTokens_ReturnsFalse()
        {
            var tokenizer = new Tokenizer("class MyClass { };", "test.h");
            tokenizer.Tokenize();
            
            tokenizer.FoundRelevantToken.Should().BeFalse();
        }

        [Fact]
        public void Tokenize_StringWithEscapedQuotes_HandlesCorrectly()
        {
            var tokenizer = new Tokenizer("\"He said \\\"Hello\\\" to me\"", "test.h");
            var tokens = tokenizer.Tokenize();
            
            tokens.Should().HaveCount(1);
            tokens[0].Type.Should().Be(TokenType.StringLiteral);
            tokens[0].Value.Should().Be("\"He said \\\"Hello\\\" to me\"");
        }

        [Fact]
        public void Tokenize_UnterminatedString_HandlesGracefully()
        {
            var tokenizer = new Tokenizer("\"unterminated string", "test.h");
            var tokens = tokenizer.Tokenize();
            
            tokens.Should().HaveCount(1);
            tokens[0].Type.Should().Be(TokenType.StringLiteral);
            tokens[0].Value.Should().Be("\"unterminated string");
        }

        [Fact]
        public void Tokenize_NestedComments_HandlesCorrectly()
        {
            var tokenizer = new Tokenizer("/* outer /* inner */ outer */", "test.h");
            var tokens = tokenizer.Tokenize();
            
            tokens.Should().HaveCount(2);
            tokens[0].Type.Should().Be(TokenType.Comment);
            tokens[0].Value.Should().Be("/* outer /* inner */");
            tokens[1].Type.Should().Be(TokenType.Whitespace);
        }
    }
}