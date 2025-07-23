using System;
using System.IO;
using System.Text;

namespace HaketonHeaderTool
{
    // Custom exception classes for better error handling
    public class HeaderToolException : Exception
    {
        public string FileName { get; }
        public int LineNumber { get; }
        public int Column { get; }
        
        public HeaderToolException(string message, string fileName = "", int lineNumber = -1, int column = -1) 
            : base(FormatMessage(message, fileName, lineNumber, column))
        {
            FileName = fileName;
            LineNumber = lineNumber;
            Column = column;
        }
        
        public HeaderToolException(string message, Exception innerException, string fileName = "", int lineNumber = -1, int column = -1)
            : base(FormatMessage(message, fileName, lineNumber, column), innerException)
        {
            FileName = fileName;
            LineNumber = lineNumber;
            Column = column;
        }
        
        private static string FormatMessage(string message, string fileName, int lineNumber, int column)
        {
            var sb = new StringBuilder(message);
            
            if (!string.IsNullOrEmpty(fileName))
            {
                sb.Append($" in file '{Path.GetFileName(fileName)}'");
            }
            
            if (lineNumber > 0)
            {
                sb.Append($" at line {lineNumber}");
                if (column > 0)
                {
                    sb.Append($", column {column}");
                }
            }
            
            return sb.ToString();
        }
    }
    
    public class ParseException : HeaderToolException
    {
        public ParseException(string message, string fileName = "", int lineNumber = -1, int column = -1)
            : base($"Parse error: {message}", fileName, lineNumber, column) { }
            
        public ParseException(string message, Exception innerException, string fileName = "", int lineNumber = -1, int column = -1)
            : base($"Parse error: {message}", innerException, fileName, lineNumber, column) { }
    }
    
    public class FileProcessingException : HeaderToolException
    {
        public FileProcessingException(string message, string fileName = "", int lineNumber = -1, int column = -1)
            : base($"File processing error: {message}", fileName, lineNumber, column) { }
            
        public FileProcessingException(string message, Exception innerException, string fileName = "", int lineNumber = -1, int column = -1)
            : base($"File processing error: {message}", innerException, fileName, lineNumber, column) { }
    }
    
    public class CodeGenerationException : HeaderToolException
    {
        public CodeGenerationException(string message, string fileName = "", int lineNumber = -1, int column = -1)
            : base($"Code generation error: {message}", fileName, lineNumber, column) { }
            
        public CodeGenerationException(string message, Exception innerException, string fileName = "", int lineNumber = -1, int column = -1)
            : base($"Code generation error: {message}", innerException, fileName, lineNumber, column) { }
    }
}