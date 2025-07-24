using System;
using System.IO;

namespace HaketonHeaderTool
{
    // Helper class to track position in source code
    public class SourcePosition
    {
        public string FileName { get; set; } = "";
        public int LineNumber { get; set; } = 1;
        public int Column { get; set; } = 1;
        public int AbsolutePosition { get; set; } = 0;
        
        public SourcePosition() { }
        
        public SourcePosition(string fileName)
        {
            FileName = fileName;
        }
        
        public SourcePosition Clone()
        {
            return new SourcePosition
            {
                FileName = FileName,
                LineNumber = LineNumber,
                Column = Column,
                AbsolutePosition = AbsolutePosition
            };
        }
        
        public override string ToString()
        {
            return $"{Path.GetFileName(FileName)}({LineNumber},{Column})";
        }
    }
    
    // Helper class to track source positions while parsing
    public class SourceTracker
    {
        private readonly string _source;
        private readonly SourcePosition _position;
        
        public SourceTracker(string source, string fileName)
        {
            _source = source;
            _position = new SourcePosition(fileName);
        }
        
        public SourcePosition CurrentPosition => _position.Clone();
        
        public void UpdatePosition(int absolutePosition)
        {
            if (absolutePosition < _position.AbsolutePosition)
            {
                // Reset to beginning if going backwards
                _position.LineNumber = 1;
                _position.Column = 1;
                _position.AbsolutePosition = 0;
            }
            
            // Update position by scanning characters
            while (_position.AbsolutePosition < absolutePosition && _position.AbsolutePosition < _source.Length)
            {
                char c = _source[_position.AbsolutePosition];
                
                if (c == '\n')
                {
                    _position.LineNumber++;
                    _position.Column = 1;
                }
                else if (c == '\r')
                {
                    // Handle Windows line endings - don't increment column for \r
                }
                else
                {
                    _position.Column++;
                }
                
                _position.AbsolutePosition++;
            }
        }
        
        public SourcePosition GetPositionAt(int absolutePosition)
        {
            var originalPos = _position.Clone();
            UpdatePosition(absolutePosition);
            var resultPos = _position.Clone();
            
            // Restore original position
            _position.LineNumber = originalPos.LineNumber;
            _position.Column = originalPos.Column;
            _position.AbsolutePosition = originalPos.AbsolutePosition;
            
            return resultPos;
        }
        
        public string GetContextAround(int position, int contextLines = 2)
        {
            var lines = _source.Split(new[] { '\r', '\n' }, StringSplitOptions.RemoveEmptyEntries);
            var pos = GetPositionAt(position);
            
            int startLine = Math.Max(0, pos.LineNumber - contextLines - 1);
            int endLine = Math.Min(lines.Length - 1, pos.LineNumber + contextLines - 1);
            
            var context = new System.Text.StringBuilder();
            for (int i = startLine; i <= endLine; i++)
            {
                bool isErrorLine = (i == pos.LineNumber - 1);
                context.AppendLine($"{i + 1,4}: {lines[i]}");
                
                if (isErrorLine)
                {
                    // Add pointer to the exact column
                    context.Append(new string(' ', 6 + pos.Column - 1));
                    context.AppendLine("^");
                }
            }
            
            return context.ToString();
        }
    }
}