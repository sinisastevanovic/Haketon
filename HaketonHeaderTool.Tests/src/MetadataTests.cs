using FluentAssertions;
using Xunit;

namespace HaketonHeaderTool.Tests
{
    public class MetadataTests
    {
        [Fact]
        public void Metadata_Creation_InitializesEmptyProperties()
        {
            var metadata = new Metadata();

            metadata.Properties.Should().NotBeNull().And.BeEmpty();
            metadata.Position.Should().BeNull();
        }

        [Fact]
        public void GetProperty_ExistingKey_ReturnsValue()
        {
            var metadata = new Metadata();
            metadata.Properties["testKey"] = "testValue";

            var result = metadata.GetProperty("testKey");

            result.Should().Be("testValue");
        }

        [Fact]
        public void GetProperty_NonExistingKey_ReturnsDefaultValue()
        {
            var metadata = new Metadata();

            var result = metadata.GetProperty("nonExistingKey", "defaultValue");

            result.Should().Be("defaultValue");
        }

        [Fact]
        public void GetProperty_NonExistingKeyNoDefault_ReturnsEmptyString()
        {
            var metadata = new Metadata();

            var result = metadata.GetProperty("nonExistingKey");

            result.Should().Be("");
        }

        [Fact]
        public void HasProperty_ExistingKey_ReturnsTrue()
        {
            var metadata = new Metadata();
            metadata.Properties["existingKey"] = "value";

            var result = metadata.HasProperty("existingKey");

            result.Should().BeTrue();
        }

        [Fact]
        public void HasProperty_NonExistingKey_ReturnsFalse()
        {
            var metadata = new Metadata();

            var result = metadata.HasProperty("nonExistingKey");

            result.Should().BeFalse();
        }

        [Fact]
        public void GetBoolProperty_TrueValue_ReturnsTrue()
        {
            var metadata = new Metadata();
            metadata.Properties["boolKey"] = "true";

            var result = metadata.GetBoolProperty("boolKey");

            result.Should().BeTrue();
        }

        [Fact]
        public void GetBoolProperty_FalseValue_ReturnsFalse()
        {
            var metadata = new Metadata();
            metadata.Properties["boolKey"] = "false";

            var result = metadata.GetBoolProperty("boolKey");

            result.Should().BeFalse();
        }

        [Fact]
        public void GetBoolProperty_InvalidValue_ReturnsTrue()
        {
            var metadata = new Metadata();
            metadata.Properties["boolKey"] = "invalidValue";

            var result = metadata.GetBoolProperty("boolKey");

            result.Should().BeTrue();
        }

        [Theory]
        [InlineData("True", true)]
        [InlineData("False", false)]
        [InlineData("TRUE", true)]
        [InlineData("FALSE", false)]
        [InlineData("1", true)] // Should not parse as bool, so returns true (flag exists)
        [InlineData("0", true)] // Should not parse as bool, so returns true (flag exists)
        public void GetBoolProperty_VariousValues_ReturnsExpectedResult(string value, bool expected)
        {
            var metadata = new Metadata();
            metadata.Properties["boolKey"] = value;

            var result = metadata.GetBoolProperty("boolKey");

            result.Should().Be(expected);
        }

        [Fact]
        public void GetBoolProperty_NonExistingKey_ReturnsDefaultValue()
        {
            var metadata = new Metadata();

            var result = metadata.GetBoolProperty("nonExistingKey", true);

            result.Should().BeTrue();
        }

        [Fact]
        public void GetBoolProperty_NonExistingKeyNoDefault_ReturnsFalse()
        {
            var metadata = new Metadata();

            var result = metadata.GetBoolProperty("nonExistingKey");

            result.Should().BeFalse();
        }

        [Fact]
        public void Properties_SetAndGet_WorksCorrectly()
        {
            var metadata = new Metadata();

            metadata.Properties["key1"] = "value1";
            metadata.Properties["key2"] = "value2";

            metadata.Properties.Should().HaveCount(2);
            metadata.Properties["key1"].Should().Be("value1");
            metadata.Properties["key2"].Should().Be("value2");
        }

        [Fact]
        public void Position_SetAndGet_WorksCorrectly()
        {
            var metadata = new Metadata();
            var position = new SourcePosition("test.h") { LineNumber = 10, Column = 5 };

            metadata.Position = position;

            metadata.Position.Should().Be(position);
        }
    }
}