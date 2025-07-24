using System.Linq;
using FluentAssertions;
using Xunit;

namespace HaketonHeaderTool.Tests
{
    public class IntegrationTests
    {
        [Fact]
        public void FullPipeline_SimpleStruct_GeneratesCorrectOutput()
        {
            var source = @"
STRUCT()
class TestComponent
{
public:
    PROPERTY()
    int health = 100;
    
    PROPERTY()
    float speed = 5.0f;
    
    FUNCTION()
    void TakeDamage(int damage);
};";

            // Tokenize
            var tokenizer = new Tokenizer(source, "TestComponent.h");
            var tokens = tokenizer.Tokenize();
            
            tokens.Should().NotBeEmpty();
            tokenizer.FoundRelevantToken.Should().BeTrue();

            // Parse
            var parser = new Parser(tokens);
            var fileNode = parser.ParseFile("TestComponent.h");
            
            fileNode.Children.Should().HaveCount(1);
            var structNode = fileNode.Children[0].Should().BeOfType<StructNode>().Subject;
            structNode.Name.Should().Be("TestComponent");
            structNode.Properties.Should().HaveCount(2);
            structNode.Functions.Should().HaveCount(1);

            // Generate
            var headerInfo = new HeaderFileInfo("C:\\Test\\", "src\\", "TestComponent");
            var generator = new CodeGenerator(headerInfo);
            var generatedFile = generator.GenerateRegistrationFile(fileNode);
            
            generatedFile.Should().NotBeNull();
            generatedFile.Content.Should().Contain("TestComponent");
            generatedFile.Content.Should().Contain("health");
            generatedFile.Content.Should().Contain("speed");
            generatedFile.Content.Should().Contain("TakeDamage");
        }

        [Fact]
        public void FullPipeline_SimpleEnum_GeneratesCorrectOutput()
        {
            var source = @"
ENUM()
enum class ComponentType
{
    Transform,
    Renderer,
    Collider = 10,
    AudioSource
};";

            // Tokenize
            var tokenizer = new Tokenizer(source, "ComponentType.h");
            var tokens = tokenizer.Tokenize();
            
            tokenizer.FoundRelevantToken.Should().BeTrue();

            // Parse
            var parser = new Parser(tokens);
            var fileNode = parser.ParseFile("ComponentType.h");
            
            fileNode.Children.Should().HaveCount(1);
            var enumNode = fileNode.Children[0].Should().BeOfType<EnumNode>().Subject;
            enumNode.Name.Should().Be("ComponentType");
            enumNode.Values.Should().HaveCount(4);
            enumNode.Values[2].Value.Should().Be("10"); // Collider = 10

            // Generate
            var headerInfo = new HeaderFileInfo("C:\\Test\\", "src\\", "ComponentType");
            var generator = new CodeGenerator(headerInfo);
            var generatedFile = generator.GenerateRegistrationFile(fileNode);
            
            generatedFile.Should().NotBeNull();
            generatedFile.Content.Should().Contain("ComponentType");
            generatedFile.Content.Should().Contain("Transform");
            generatedFile.Content.Should().Contain("Renderer");
            generatedFile.Content.Should().Contain("Collider");
            generatedFile.Content.Should().Contain("AudioSource");
        }

        [Fact]
        public void FullPipeline_ComplexStructWithInheritance_HandlesCorrectly()
        {
            var source = @"
STRUCT()
class RenderComponent
{
private:
    PROPERTY()
    FVec3 position;

protected:
    PROPERTY()
    bool isVisible = true;

public:
    PROPERTY()
    std::string texturePath;
    
    FUNCTION()
    void SetPosition(const FVec3& pos);
    
    FUNCTION()
    FVec3 GetPosition() const;
};";

            // Full pipeline
            var tokenizer = new Tokenizer(source, "RenderComponent.h");
            var tokens = tokenizer.Tokenize();
            var parser = new Parser(tokens);
            var fileNode = parser.ParseFile("RenderComponent.h");
            
            var structNode = fileNode.Children[0].Should().BeOfType<StructNode>().Subject;
            structNode.Name.Should().Be("RenderComponent");
            structNode.BaseClass.Should().BeNull();
            structNode.Properties.Should().HaveCount(3);
            structNode.Functions.Should().HaveCount(2);
            
            // Check properties exist (parser doesn't track access modifiers)
            var positionProp = structNode.Properties.FirstOrDefault(p => p.Name == "position");
            positionProp.Should().NotBeNull();
            positionProp.Type.Should().Be("FVec3");
            
            var visibleProp = structNode.Properties.FirstOrDefault(p => p.Name == "isVisible");
            visibleProp.Should().NotBeNull();
            visibleProp.Type.Should().Be("bool");
            
            var textureProp = structNode.Properties.FirstOrDefault(p => p.Name == "texturePath");
            textureProp.Should().NotBeNull();
            textureProp.Type.Should().Be("std::string");

            // Generate
            var headerInfo = new HeaderFileInfo("C:\\Test\\", "src\\", "RenderComponent");
            var generator = new CodeGenerator(headerInfo);
            var generatedFile = generator.GenerateRegistrationFile(fileNode);
            
            generatedFile.Should().NotBeNull();
            generatedFile.Content.Should().Contain("RenderComponent");
        }

        [Fact]
        public void FullPipeline_MultipleDeclarations_HandlesBoth()
        {
            var source = @"
ENUM()
enum class LogLevel
{
    Info,
    Warning,
    Error
};

STRUCT()
class Logger
{
public:
    PROPERTY()
    LogLevel currentLevel = LogLevel::Info;
    
    FUNCTION()
    void Log(const std::string& message, LogLevel level);
};";

            // Full pipeline
            var tokenizer = new Tokenizer(source, "Logger.h");
            var tokens = tokenizer.Tokenize();
            var parser = new Parser(tokens);
            var fileNode = parser.ParseFile("Logger.h");
            
            fileNode.Children.Should().HaveCount(2);
            fileNode.Children[0].Should().BeOfType<EnumNode>();
            fileNode.Children[1].Should().BeOfType<StructNode>();
            
            var enumNode = (EnumNode)fileNode.Children[0];
            enumNode.Name.Should().Be("LogLevel");
            enumNode.Values.Should().HaveCount(3);
            
            var structNode = (StructNode)fileNode.Children[1];
            structNode.Name.Should().Be("Logger");
            structNode.Properties.Should().HaveCount(1);
            structNode.Functions.Should().HaveCount(1);

            // Generate
            var headerInfo = new HeaderFileInfo("C:\\Test\\", "src\\", "Logger");
            var generator = new CodeGenerator(headerInfo);
            var generatedFile = generator.GenerateRegistrationFile(fileNode);
            
            generatedFile.Should().NotBeNull();
            generatedFile.Content.Should().Contain("LogLevel");
            generatedFile.Content.Should().Contain("Logger");
        }

        [Fact]
        public void FullPipeline_WithComments_IgnoresComments()
        {
            var source = @"
// This is a test component
STRUCT()
class TestComponent  // Component for testing
{
public:
    // Health property
    PROPERTY()
    int health; /* Default health value */
    
    /* 
     * Multi-line comment
     * about this function
     */
    FUNCTION()
    void DoSomething(); // Does something important
};";

            var tokenizer = new Tokenizer(source, "TestComponent.h");
            var tokens = tokenizer.Tokenize();
            var parser = new Parser(tokens);
            var fileNode = parser.ParseFile("TestComponent.h");
            
            fileNode.Children.Should().HaveCount(1);
            var structNode = fileNode.Children[0].Should().BeOfType<StructNode>().Subject;
            structNode.Name.Should().Be("TestComponent");
            structNode.Properties.Should().HaveCount(1);
            structNode.Functions.Should().HaveCount(1);
        }

        [Fact]
        public void FullPipeline_NoReflectionTokens_ReturnsNull()
        {
            var source = @"
class RegularClass
{
public:
    int normalProperty;
    void normalFunction();
};";

            var tokenizer = new Tokenizer(source, "RegularClass.h");
            tokenizer.FoundRelevantToken.Should().BeFalse();
            
            var tokens = tokenizer.Tokenize();
            var parser = new Parser(tokens);
            var fileNode = parser.ParseFile("RegularClass.h");
            
            var headerInfo = new HeaderFileInfo("C:\\Test\\", "src\\", "RegularClass");
            var generator = new CodeGenerator(headerInfo);
            var generatedFile = generator.GenerateRegistrationFile(fileNode);
            
            generatedFile.Should().BeNull();
        }
    }
}