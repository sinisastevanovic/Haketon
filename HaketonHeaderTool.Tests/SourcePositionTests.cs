using FluentAssertions;
using Xunit;

namespace HaketonHeaderTool.Tests
{
    public class SourcePositionTests
    {
        [Fact]
        public void SourcePosition_DefaultCreation_SetsDefaultValues()
        {
            var position = new SourcePosition();

            position.FileName.Should().Be("");
            position.LineNumber.Should().Be(1);
            position.Column.Should().Be(1);
            position.AbsolutePosition.Should().Be(0);
        }

        [Fact]
        public void SourcePosition_CreationWithFileName_SetsFileName()
        {
            var position = new SourcePosition("test.h");

            position.FileName.Should().Be("test.h");
            position.LineNumber.Should().Be(1);
            position.Column.Should().Be(1);
            position.AbsolutePosition.Should().Be(0);
        }

        [Fact]
        public void Clone_CreatesExactCopy()
        {
            var position = new SourcePosition("test.h")
            {
                LineNumber = 5,
                Column = 10,
                AbsolutePosition = 42
            };

            var cloned = position.Clone();

            cloned.Should().NotBeSameAs(position);
            cloned.FileName.Should().Be(position.FileName);
            cloned.LineNumber.Should().Be(position.LineNumber);
            cloned.Column.Should().Be(position.Column);
            cloned.AbsolutePosition.Should().Be(position.AbsolutePosition);
        }

        [Fact]
        public void ToString_FormatsCorrectly()
        {
            var position = new SourcePosition("D:\\Test\\file.h")
            {
                LineNumber = 15,
                Column = 8
            };

            var result = position.ToString();

            result.Should().Be("file.h(15,8)");
        }

        [Fact]
        public void ToString_WithFullPath_ShowsOnlyFileName()
        {
            var position = new SourcePosition("C:\\Projects\\MyProject\\headers\\test.h")
            {
                LineNumber = 1,
                Column = 1
            };

            var result = position.ToString();

            result.Should().Be("test.h(1,1)");
        }
    }

    public class SourceTrackerTests
    {
        [Fact]
        public void SourceTracker_Creation_SetsInitialState()
        {
            var source = "test content";
            var tracker = new SourceTracker(source, "test.h");

            var position = tracker.CurrentPosition;
            position.FileName.Should().Be("test.h");
            position.LineNumber.Should().Be(1);
            position.Column.Should().Be(1);
            position.AbsolutePosition.Should().Be(0);
        }

        [Fact]
        public void UpdatePosition_SingleLine_UpdatesColumnCorrectly()
        {
            var source = "hello world";
            var tracker = new SourceTracker(source, "test.h");

            tracker.UpdatePosition(5);

            var position = tracker.CurrentPosition;
            position.LineNumber.Should().Be(1);
            position.Column.Should().Be(6); // 1-based, so position 5 = column 6
            position.AbsolutePosition.Should().Be(5);
        }

        [Fact]
        public void UpdatePosition_MultipleLines_UpdatesLineAndColumnCorrectly()
        {
            var source = "line1\nline2\nline3";
            var tracker = new SourceTracker(source, "test.h");

            tracker.UpdatePosition(8); // Position after "line1\nli"

            var position = tracker.CurrentPosition;
            position.LineNumber.Should().Be(2);
            position.Column.Should().Be(3); // "li" = column 3
            position.AbsolutePosition.Should().Be(8);
        }

        [Fact]
        public void UpdatePosition_WindowsLineEndings_HandlesCorrectly()
        {
            var source = "line1\r\nline2\r\nline3";
            var tracker = new SourceTracker(source, "test.h");

            tracker.UpdatePosition(9); // Position after "line1\r\nli"

            var position = tracker.CurrentPosition;
            position.LineNumber.Should().Be(2);
            position.Column.Should().Be(3); // "li" = column 3
            position.AbsolutePosition.Should().Be(9);
        }

        [Fact]
        public void UpdatePosition_BackwardsMovement_ResetsAndRecalculates()
        {
            var source = "line1\nline2\nline3";
            var tracker = new SourceTracker(source, "test.h");

            // Move forward first
            tracker.UpdatePosition(10);
            var forwardPosition = tracker.CurrentPosition;
            forwardPosition.LineNumber.Should().Be(2);

            // Move backwards
            tracker.UpdatePosition(3);
            var backwardPosition = tracker.CurrentPosition;
            backwardPosition.LineNumber.Should().Be(1);
            backwardPosition.Column.Should().Be(4);
            backwardPosition.AbsolutePosition.Should().Be(3);
        }

        [Fact]
        public void GetPositionAt_DoesNotAffectCurrentPosition()
        {
            var source = "line1\nline2\nline3";
            var tracker = new SourceTracker(source, "test.h");

            var originalPosition = tracker.CurrentPosition;
            var queriedPosition = tracker.GetPositionAt(8);

            // Queried position should be correct
            queriedPosition.LineNumber.Should().Be(2);
            queriedPosition.Column.Should().Be(3);
            queriedPosition.AbsolutePosition.Should().Be(8);

            // Current position should remain unchanged
            var currentPosition = tracker.CurrentPosition;
            currentPosition.LineNumber.Should().Be(originalPosition.LineNumber);
            currentPosition.Column.Should().Be(originalPosition.Column);
            currentPosition.AbsolutePosition.Should().Be(originalPosition.AbsolutePosition);
        }

        [Fact]
        public void GetContextAround_SingleLine_ReturnsContextWithPointer()
        {
            var source = "This is a test line";
            var tracker = new SourceTracker(source, "test.h");

            var context = tracker.GetContextAround(8, 1); // Position at 'a'

            context.Should().Contain("1: This is a test line");
            context.Should().Contain("^");
        }

        [Fact]
        public void GetContextAround_MultipleLines_ReturnsContextLines()
        {
            var source = "line1\nline2\nline3\nline4\nline5";
            var tracker = new SourceTracker(source, "test.h");

            var context = tracker.GetContextAround(9, 1); // Position in line2

            context.Should().Contain("1: line1");
            context.Should().Contain("2: line2");
            context.Should().Contain("3: line3");
            context.Should().Contain("^");
        }

        [Fact]
        public void GetContextAround_AtBeginning_HandlesGracefully()
        {
            var source = "line1\nline2\nline3";
            var tracker = new SourceTracker(source, "test.h");

            var context = tracker.GetContextAround(0, 2);

            context.Should().Contain("1: line1");
            context.Should().NotContain("0:");
        }

        [Fact]
        public void GetContextAround_AtEnd_HandlesGracefully()
        {
            var source = "line1\nline2\nline3";
            var tracker = new SourceTracker(source, "test.h");

            var context = tracker.GetContextAround(source.Length - 1, 2);

            context.Should().Contain("line3");
            context.Should().NotContain("4:");
        }
    }
}