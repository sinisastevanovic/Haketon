using System;
using System.Linq;
using FluentAssertions;
using Xunit;

namespace HaketonHeaderTool.Tests
{
    public class CodeGeneratorTests
    {
        private HeaderFileInfo CreateTestHeaderInfo()
        {
            return new HeaderFileInfo("C:\\Test\\", "src\\", "TestFile");
        }

        private SourcePosition CreateTestPosition()
        {
            return new SourcePosition("TestFile.h") { LineNumber = 1, Column = 1 };
        }

        [Fact]
        public void CodeGenerator_Creation_WithValidHeaderInfo_Succeeds()
        {
            var headerInfo = CreateTestHeaderInfo();

            var generator = new CodeGenerator(headerInfo);

            generator.Should().NotBeNull();
        }

        [Fact]
        public void CodeGenerator_Creation_WithNullHeaderInfo_ThrowsArgumentNullException()
        {
            HeaderFileInfo headerInfo = null;

            var action = () => new CodeGenerator(headerInfo);

            action.Should().Throw<ArgumentNullException>().WithParameterName("headerFileInfo");
        }

        [Fact]
        public void GenerateRegistrationFile_WithNullFileNode_ThrowsArgumentNullException()
        {
            var headerInfo = CreateTestHeaderInfo();
            var generator = new CodeGenerator(headerInfo);

            var action = () => generator.GenerateRegistrationFile(null);

            action.Should().Throw<ArgumentNullException>().WithParameterName("fileNode");
        }

        [Fact]
        public void GenerateRegistrationFile_WithEmptyFileNode_ReturnsNull()
        {
            var headerInfo = CreateTestHeaderInfo();
            var generator = new CodeGenerator(headerInfo);
            var fileNode = new FileNode("TestFile.h", CreateTestPosition());

            var result = generator.GenerateRegistrationFile(fileNode);

            result.Should().BeNull();
        }

        [Fact]
        public void GenerateRegistrationFile_WithStructNode_ReturnsGeneratedFile()
        {
            var headerInfo = CreateTestHeaderInfo();
            var generator = new CodeGenerator(headerInfo);
            var fileNode = new FileNode("TestFile.h", CreateTestPosition());
            
            var structNode = new StructNode("TestStruct", CreateTestPosition());
            structNode.Metadata = new Metadata();
            structNode.Metadata.Properties["Component"] = "true";
            structNode.IsComponent = true;
            
            fileNode.Children.Add(structNode);

            var result = generator.GenerateRegistrationFile(fileNode);

            result.Should().NotBeNull();
            result.Content.Should().NotBeNullOrEmpty();
            result.FileName.Should().Contain("TestFile");
        }

        [Fact]
        public void GenerateRegistrationFile_WithEnumNode_ReturnsGeneratedFile()
        {
            var headerInfo = CreateTestHeaderInfo();
            var generator = new CodeGenerator(headerInfo);
            var fileNode = new FileNode("TestFile.h", CreateTestPosition());
            
            var enumNode = new EnumNode("TestEnum", CreateTestPosition());
            enumNode.Metadata = new Metadata();
            enumNode.Values.Add(new EnumValueNode("Value1", CreateTestPosition()));
            enumNode.Values.Add(new EnumValueNode("Value2", CreateTestPosition()));
            
            fileNode.Children.Add(enumNode);

            var result = generator.GenerateRegistrationFile(fileNode);

            result.Should().NotBeNull();
            result.Content.Should().NotBeNullOrEmpty();
            result.Content.Should().Contain("TestEnum");
        }

        [Fact]
        public void GenerateRegistrationFile_WithStructWithProperties_GeneratesPropertyRegistration()
        {
            var headerInfo = CreateTestHeaderInfo();
            var generator = new CodeGenerator(headerInfo);
            var fileNode = new FileNode("TestFile.h", CreateTestPosition());
            
            var structNode = new StructNode("TestStruct", CreateTestPosition());
            structNode.Metadata = new Metadata();
            structNode.Metadata.Properties["Component"] = "true";
            structNode.IsComponent = true;
            
            var property = new PropertyNode("TestProperty", "int", CreateTestPosition());
            property.Metadata = new Metadata();
            structNode.Properties.Add(property);
            
            fileNode.Children.Add(structNode);

            var result = generator.GenerateRegistrationFile(fileNode);

            result.Should().NotBeNull();
            result.Content.Should().Contain("TestProperty");
            result.Content.Should().Contain("int");
        }

        [Fact]
        public void GenerateRegistrationFile_WithStructWithFunctions_GeneratesFunctionRegistration()
        {
            var headerInfo = CreateTestHeaderInfo();
            var generator = new CodeGenerator(headerInfo);
            var fileNode = new FileNode("TestFile.h", CreateTestPosition());
            
            var structNode = new StructNode("TestStruct", CreateTestPosition());
            structNode.Metadata = new Metadata();
            structNode.Metadata.Properties["Component"] = "true";
            structNode.IsComponent = true;
            
            var function = new FunctionNode("TestFunction", "void", CreateTestPosition());
            function.Metadata = new Metadata();
            structNode.Functions.Add(function);
            
            fileNode.Children.Add(structNode);

            var result = generator.GenerateRegistrationFile(fileNode);

            result.Should().NotBeNull();
            result.Content.Should().Contain("TestFunction");
        }

        [Fact]
        public void GenerateRegistrationFile_WithMultipleNodes_GeneratesAllRegistrations()
        {
            var headerInfo = CreateTestHeaderInfo();
            var generator = new CodeGenerator(headerInfo);
            var fileNode = new FileNode("TestFile.h", CreateTestPosition());
            
            // Add struct
            var structNode = new StructNode("TestStruct", CreateTestPosition());
            structNode.Metadata = new Metadata();
            structNode.Metadata.Properties["Component"] = "true";
            structNode.IsComponent = true;
            fileNode.Children.Add(structNode);
            
            // Add enum
            var enumNode = new EnumNode("TestEnum", CreateTestPosition());
            enumNode.Metadata = new Metadata();
            enumNode.Values.Add(new EnumValueNode("Value1", CreateTestPosition()));
            fileNode.Children.Add(enumNode);

            var result = generator.GenerateRegistrationFile(fileNode);

            result.Should().NotBeNull();
            result.Content.Should().Contain("TestStruct");
            result.Content.Should().Contain("TestEnum");
        }

        [Fact]
        public void GenerateRegistrationFile_WithStructWithInheritance_HandlesBaseClass()
        {
            var headerInfo = CreateTestHeaderInfo();
            var generator = new CodeGenerator(headerInfo);
            var fileNode = new FileNode("TestFile.h", CreateTestPosition());
            
            var structNode = new StructNode("DerivedStruct", CreateTestPosition());
            structNode.Metadata = new Metadata();
            structNode.Metadata.Properties["Component"] = "true";
            structNode.IsComponent = true;
            structNode.BaseClass = "BaseStruct";
            
            fileNode.Children.Add(structNode);

            var result = generator.GenerateRegistrationFile(fileNode);

            result.Should().NotBeNull();
            result.Content.Should().Contain("DerivedStruct");
        }

        [Fact]
        public void GenerateRegistrationFile_WithEnumWithValues_GeneratesAllEnumValues()
        {
            var headerInfo = CreateTestHeaderInfo();
            var generator = new CodeGenerator(headerInfo);
            var fileNode = new FileNode("TestFile.h", CreateTestPosition());
            
            var enumNode = new EnumNode("TestEnum", CreateTestPosition());
            enumNode.Metadata = new Metadata();
            
            var value1 = new EnumValueNode("FirstValue", CreateTestPosition());
            var value2 = new EnumValueNode("SecondValue", CreateTestPosition());
            value2.Value = "5"; // With explicit value
            
            enumNode.Values.Add(value1);
            enumNode.Values.Add(value2);
            
            fileNode.Children.Add(enumNode);

            var result = generator.GenerateRegistrationFile(fileNode);

            result.Should().NotBeNull();
            result.Content.Should().Contain("FirstValue");
            result.Content.Should().Contain("SecondValue");
        }

        [Fact]
        public void GenerateRegistrationFile_WithPropertyMetadata_GeneratesMetadataInfo()
        {
            var headerInfo = CreateTestHeaderInfo();
            var generator = new CodeGenerator(headerInfo);
            var fileNode = new FileNode("TestFile.h", CreateTestPosition());
            
            var structNode = new StructNode("TestStruct", CreateTestPosition());
            structNode.Metadata = new Metadata();
            structNode.Metadata.Properties["Component"] = "true";
            structNode.IsComponent = true;
            
            var property = new PropertyNode("TestProperty", "float", CreateTestPosition());
            property.Metadata = new Metadata();
            property.Metadata.Properties["Range"] = "0,100";
            property.Metadata.Properties["DisplayName"] = "Test Property";
            
            structNode.Properties.Add(property);
            fileNode.Children.Add(structNode);

            var result = generator.GenerateRegistrationFile(fileNode);

            result.Should().NotBeNull();
            result.Content.Should().Contain("TestProperty");
            result.Content.Should().Contain("float");
        }
    }
}