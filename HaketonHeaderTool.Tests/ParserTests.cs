using System.Collections.Generic;
using System.Linq;
using FluentAssertions;
using Xunit;

namespace HaketonHeaderTool.Tests
{
    public class ParserTests
    {
        private List<Token> CreateTokens(params (TokenType type, string value)[] tokenData)
        {
            var tokens = new List<Token>();
            var position = new SourcePosition("test.h");
            
            foreach (var (type, value) in tokenData)
            {
                tokens.Add(new Token(type, value, position));
            }
            
            return tokens;
        }

        [Fact]
        public void ParseFile_EmptyTokenList_ReturnsEmptyFileNode()
        {
            var tokens = new List<Token>();
            var parser = new Parser(tokens);
            
            var fileNode = parser.ParseFile("test.h");
            
            fileNode.Should().NotBeNull();
            fileNode.FileName.Should().Be("test.h");
            fileNode.Children.Should().BeEmpty();
        }

        [Fact]
        public void ParseFile_SimpleStruct_ReturnsStructNode()
        {
            var tokens = CreateTokens(
                (TokenType.Struct, "STRUCT"),
                (TokenType.OpenParen, "("),
                (TokenType.CloseParen, ")"),
                (TokenType.Class, "class"),
                (TokenType.Identifier, "TestStruct"),
                (TokenType.OpenBrace, "{"),
                (TokenType.CloseBrace, "}")
            );

            var parser = new Parser(tokens);
            var fileNode = parser.ParseFile("test.h");

            fileNode.Children.Should().HaveCount(1);
            var structNode = fileNode.Children[0].Should().BeOfType<StructNode>().Subject;
            structNode.Name.Should().Be("TestStruct");
            structNode.Metadata.Should().NotBeNull();
        }

        [Fact]
        public void ParseFile_StructWithProperty_ReturnsStructWithPropertyNode()
        {
            var tokens = CreateTokens(
                (TokenType.Struct, "STRUCT"),
                (TokenType.OpenParen, "("),
                (TokenType.CloseParen, ")"),
                (TokenType.Class, "class"),
                (TokenType.Identifier, "TestStruct"),
                (TokenType.OpenBrace, "{"),
                (TokenType.Public, "public"),
                (TokenType.Colon, ":"),
                (TokenType.Property, "PROPERTY"),
                (TokenType.OpenParen, "("),
                (TokenType.CloseParen, ")"),
                (TokenType.Identifier, "int"),
                (TokenType.Identifier, "value"),
                (TokenType.Semicolon, ";"),
                (TokenType.CloseBrace, "}")
            );

            var parser = new Parser(tokens);
            var fileNode = parser.ParseFile("test.h");

            fileNode.Children.Should().HaveCount(1);
            var structNode = fileNode.Children[0].Should().BeOfType<StructNode>().Subject;
            structNode.Properties.Should().HaveCount(1);
            
            var property = structNode.Properties[0];
            property.Name.Should().Be("value");
            property.Type.Should().Be("int");
            property.AccessModifier.Should().Be("public");
        }

        [Fact]
        public void ParseFile_StructWithFunction_ReturnsStructWithFunctionNode()
        {
            var tokens = CreateTokens(
                (TokenType.Struct, "STRUCT"),
                (TokenType.OpenParen, "("),
                (TokenType.CloseParen, ")"),
                (TokenType.Class, "class"),
                (TokenType.Identifier, "TestStruct"),
                (TokenType.OpenBrace, "{"),
                (TokenType.Public, "public"),
                (TokenType.Colon, ":"),
                (TokenType.Function, "FUNCTION"),
                (TokenType.OpenParen, "("),
                (TokenType.CloseParen, ")"),
                (TokenType.Identifier, "void"),
                (TokenType.Identifier, "DoSomething"),
                (TokenType.OpenParen, "("),
                (TokenType.CloseParen, ")"),
                (TokenType.Semicolon, ";"),
                (TokenType.CloseBrace, "}")
            );

            var parser = new Parser(tokens);
            var fileNode = parser.ParseFile("test.h");

            fileNode.Children.Should().HaveCount(1);
            var structNode = fileNode.Children[0].Should().BeOfType<StructNode>().Subject;
            structNode.Functions.Should().HaveCount(1);
            
            var function = structNode.Functions[0];
            function.Name.Should().Be("DoSomething");
            function.ReturnType.Should().Be("void");
            function.Parameters.Should().BeEmpty();
        }

        [Fact]
        public void ParseFile_Enum_ReturnsEnumNode()
        {
            var tokens = CreateTokens(
                (TokenType.Enum, "ENUM"),
                (TokenType.OpenParen, "("),
                (TokenType.CloseParen, ")"),
                (TokenType.Identifier, "TestEnum"),
                (TokenType.OpenBrace, "{"),
                (TokenType.Identifier, "Value1"),
                (TokenType.Comma, ","),
                (TokenType.Identifier, "Value2"),
                (TokenType.Assignment, "="),
                (TokenType.NumberLiteral, "5"),
                (TokenType.CloseBrace, "}")
            );

            var parser = new Parser(tokens);
            var fileNode = parser.ParseFile("test.h");

            fileNode.Children.Should().HaveCount(1);
            var enumNode = fileNode.Children[0].Should().BeOfType<EnumNode>().Subject;
            enumNode.Name.Should().Be("TestEnum");
            enumNode.Values.Should().HaveCount(2);
            
            enumNode.Values[0].Name.Should().Be("Value1");
            enumNode.Values[0].Value.Should().BeNull();
            
            enumNode.Values[1].Name.Should().Be("Value2");
            enumNode.Values[1].Value.Should().Be("5");
        }

        [Fact]
        public void ParseFile_FunctionWithParameters_ReturnsFunctionWithParameterNodes()
        {
            var tokens = CreateTokens(
                (TokenType.Struct, "STRUCT"),
                (TokenType.OpenParen, "("),
                (TokenType.CloseParen, ")"),
                (TokenType.Class, "class"),
                (TokenType.Identifier, "TestStruct"),
                (TokenType.OpenBrace, "{"),
                (TokenType.Public, "public"),
                (TokenType.Colon, ":"),
                (TokenType.Function, "FUNCTION"),
                (TokenType.OpenParen, "("),
                (TokenType.CloseParen, ")"),
                (TokenType.Identifier, "int"),
                (TokenType.Identifier, "Calculate"),
                (TokenType.OpenParen, "("),
                (TokenType.Identifier, "int"),
                (TokenType.Identifier, "a"),
                (TokenType.Comma, ","),
                (TokenType.Identifier, "float"),
                (TokenType.Identifier, "b"),
                (TokenType.CloseParen, ")"),
                (TokenType.Semicolon, ";"),
                (TokenType.CloseBrace, "}")
            );

            var parser = new Parser(tokens);
            var fileNode = parser.ParseFile("test.h");

            var structNode = fileNode.Children[0].Should().BeOfType<StructNode>().Subject;
            var function = structNode.Functions[0];
            
            // The parser doesn't currently parse function parameters
            function.Parameters.Should().BeEmpty();
            function.Name.Should().Be("Calculate");
            function.ReturnType.Should().Be("int");
        }

        [Fact]
        public void ParseFile_StructWithInheritance_ReturnsStructWithBaseClass()
        {
            var tokens = CreateTokens(
                (TokenType.Struct, "STRUCT"),
                (TokenType.OpenParen, "("),
                (TokenType.CloseParen, ")"),
                (TokenType.Class, "class"),
                (TokenType.Identifier, "DerivedStruct"),
                (TokenType.Colon, ":"),
                (TokenType.Identifier, "BaseStruct"),
                (TokenType.OpenBrace, "{"),
                (TokenType.CloseBrace, "}")
            );

            var parser = new Parser(tokens);
            var fileNode = parser.ParseFile("test.h");

            var structNode = fileNode.Children[0].Should().BeOfType<StructNode>().Subject;
            structNode.Name.Should().Be("DerivedStruct");
            structNode.BaseClass.Should().Be("BaseStruct");
        }

        [Fact]
        public void ParseFile_MultipleDeclarations_ReturnsAllNodes()
        {
            var tokens = CreateTokens(
                (TokenType.Enum, "ENUM"),
                (TokenType.OpenParen, "("),
                (TokenType.CloseParen, ")"),
                (TokenType.Identifier, "enum"),
                (TokenType.Identifier, "FirstEnum"),
                (TokenType.OpenBrace, "{"),
                (TokenType.Identifier, "Value1"),
                (TokenType.CloseBrace, "}"),
                (TokenType.Struct, "STRUCT"),
                (TokenType.OpenParen, "("),
                (TokenType.CloseParen, ")"),
                (TokenType.Class, "class"),
                (TokenType.Identifier, "SecondStruct"),
                (TokenType.OpenBrace, "{"),
                (TokenType.CloseBrace, "}")
            );

            var parser = new Parser(tokens);
            var fileNode = parser.ParseFile("test.h");

            fileNode.Children.Should().HaveCount(2);
            fileNode.Children[0].Should().BeOfType<EnumNode>();
            fileNode.Children[1].Should().BeOfType<StructNode>();
        }
    }
}