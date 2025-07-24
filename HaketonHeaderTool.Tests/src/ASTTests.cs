using FluentAssertions;
using Xunit;

namespace HaketonHeaderTool.Tests
{
    public class ASTTests
    {
        [Fact]
        public void StructNode_Creation_SetsPropertiesCorrectly()
        {
            var position = new SourcePosition("test.h") { LineNumber = 5, Column = 10 };
            var structNode = new StructNode("TestStruct", position);

            structNode.Name.Should().Be("TestStruct");
            structNode.Position.Should().Be(position);
            structNode.BaseClass.Should().BeNull();
            structNode.Properties.Should().NotBeNull().And.BeEmpty();
            structNode.Functions.Should().NotBeNull().And.BeEmpty();
            structNode.IsComponent.Should().BeFalse();
        }

        [Fact]
        public void PropertyNode_Creation_SetsPropertiesCorrectly()
        {
            var position = new SourcePosition("test.h") { LineNumber = 10, Column = 5 };
            var propertyNode = new PropertyNode("TestProperty", "int", position);

            propertyNode.Name.Should().Be("TestProperty");
            propertyNode.Type.Should().Be("int");
            propertyNode.Position.Should().Be(position);
            propertyNode.AccessModifier.Should().Be("public");
        }

        [Fact]
        public void FunctionNode_Creation_SetsPropertiesCorrectly()
        {
            var position = new SourcePosition("test.h") { LineNumber = 15, Column = 8 };
            var functionNode = new FunctionNode("TestFunction", "void", position);

            functionNode.Name.Should().Be("TestFunction");
            functionNode.ReturnType.Should().Be("void");
            functionNode.Position.Should().Be(position);
            functionNode.AccessModifier.Should().Be("public");
            functionNode.Parameters.Should().NotBeNull().And.BeEmpty();
        }

        [Fact]
        public void EnumNode_Creation_SetsPropertiesCorrectly()
        {
            var position = new SourcePosition("test.h") { LineNumber = 20, Column = 1 };
            var enumNode = new EnumNode("TestEnum", position);

            enumNode.Name.Should().Be("TestEnum");
            enumNode.Position.Should().Be(position);
            enumNode.Values.Should().NotBeNull().And.BeEmpty();
        }

        [Fact]
        public void EnumValueNode_Creation_SetsPropertiesCorrectly()
        {
            var position = new SourcePosition("test.h") { LineNumber = 22, Column = 4 };
            var enumValueNode = new EnumValueNode("TestValue", position);

            enumValueNode.Name.Should().Be("TestValue");
            enumValueNode.Position.Should().Be(position);
            enumValueNode.Value.Should().BeNull();
        }

        [Fact]
        public void ParameterNode_Creation_SetsPropertiesCorrectly()
        {
            var position = new SourcePosition("test.h") { LineNumber = 25, Column = 12 };
            var parameterNode = new ParameterNode("param", "float", position);

            parameterNode.Name.Should().Be("param");
            parameterNode.Type.Should().Be("float");
            parameterNode.Position.Should().Be(position);
        }

        [Fact]
        public void ForwardDeclarationNode_Creation_SetsPropertiesCorrectly()
        {
            var position = new SourcePosition("test.h") { LineNumber = 1, Column = 1 };
            var forwardDeclNode = new ForwardDeclarationNode("TestClass", "class", position);

            forwardDeclNode.Name.Should().Be("TestClass");
            forwardDeclNode.Type.Should().Be("class");
            forwardDeclNode.Position.Should().Be(position);
        }

        [Fact]
        public void FileNode_Creation_SetsPropertiesCorrectly()
        {
            var position = new SourcePosition("test.h");
            var fileNode = new FileNode("test.h", position);

            fileNode.FileName.Should().Be("test.h");
            fileNode.Position.Should().Be(position);
            fileNode.Children.Should().NotBeNull().And.BeEmpty();
        }

        [Fact]
        public void FileNode_AddChildren_ContainsAllChildren()
        {
            var position = new SourcePosition("test.h");
            var fileNode = new FileNode("test.h", position);
            var structNode = new StructNode("TestStruct", position);
            var enumNode = new EnumNode("TestEnum", position);

            fileNode.Children.Add(structNode);
            fileNode.Children.Add(enumNode);

            fileNode.Children.Should().HaveCount(2);
            fileNode.Children.Should().Contain(structNode);
            fileNode.Children.Should().Contain(enumNode);
        }

        [Fact]
        public void StructNode_AddPropertiesAndFunctions_ContainsAllMembers()
        {
            var position = new SourcePosition("test.h");
            var structNode = new StructNode("TestStruct", position);
            var property = new PropertyNode("prop", "int", position);
            var function = new FunctionNode("func", "void", position);

            structNode.Properties.Add(property);
            structNode.Functions.Add(function);

            structNode.Properties.Should().HaveCount(1);
            structNode.Properties.Should().Contain(property);
            structNode.Functions.Should().HaveCount(1);
            structNode.Functions.Should().Contain(function);
        }

        [Fact]
        public void EnumNode_AddValues_ContainsAllValues()
        {
            var position = new SourcePosition("test.h");
            var enumNode = new EnumNode("TestEnum", position);
            var value1 = new EnumValueNode("Value1", position);
            var value2 = new EnumValueNode("Value2", position);

            enumNode.Values.Add(value1);
            enumNode.Values.Add(value2);

            enumNode.Values.Should().HaveCount(2);
            enumNode.Values.Should().Contain(value1);
            enumNode.Values.Should().Contain(value2);
        }

        [Fact]
        public void FunctionNode_AddParameters_ContainsAllParameters()
        {
            var position = new SourcePosition("test.h");
            var functionNode = new FunctionNode("TestFunction", "int", position);
            var param1 = new ParameterNode("param1", "int", position);
            var param2 = new ParameterNode("param2", "float", position);

            functionNode.Parameters.Add(param1);
            functionNode.Parameters.Add(param2);

            functionNode.Parameters.Should().HaveCount(2);
            functionNode.Parameters.Should().Contain(param1);
            functionNode.Parameters.Should().Contain(param2);
        }
    }
}