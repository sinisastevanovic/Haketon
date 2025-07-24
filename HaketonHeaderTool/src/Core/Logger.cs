using System;
using System.IO;

namespace HaketonHeaderTool
{
    public enum LogLevel
    {
        Debug,
        Info,
        Warning,
        Error,
        Fatal
    }
    
    public static class Logger
    {
        private static LogLevel _minLevel = LogLevel.Info;
        private static bool _showTimestamps = true;
        private static StreamWriter _logFile = null;
        
        public static LogLevel MinLevel
        {
            get => _minLevel;
            set => _minLevel = value;
        }
        
        public static bool ShowTimestamps
        {
            get => _showTimestamps;
            set => _showTimestamps = value;
        }
        
        public static void SetLogFile(string filePath)
        {
            _logFile?.Close();
            _logFile = new StreamWriter(filePath, append: true);
        }
        
        public static void Debug(string message)
        {
            Log(LogLevel.Debug, message);
        }
        
        public static void Info(string message)
        {
            Log(LogLevel.Info, message);
        }
        
        public static void Warning(string message)
        {
            Log(LogLevel.Warning, message);
        }
        
        public static void Warning(string message, string fileName, int lineNumber)
        {
            var formattedMessage = $"{message} in {Path.GetFileName(fileName)} at line {lineNumber}";
            Log(LogLevel.Warning, formattedMessage);
        }
        
        public static void Error(string message)
        {
            Log(LogLevel.Error, message);
        }
        
        public static void Error(Exception ex, string message = "")
        {
            var fullMessage = string.IsNullOrEmpty(message) ? ex.Message : $"{message}: {ex.Message}";
            Log(LogLevel.Error, fullMessage);
            
            if (ex is HeaderToolException htEx)
            {
                Log(LogLevel.Error, $"  File: {htEx.FileName}");
                Log(LogLevel.Error, $"  Line: {htEx.LineNumber}, Column: {htEx.Column}");
            }
            
            if (_minLevel <= LogLevel.Debug && ex.StackTrace != null)
            {
                Log(LogLevel.Debug, $"Stack trace:\n{ex.StackTrace}");
            }
        }
        
        public static void Fatal(string message)
        {
            Log(LogLevel.Fatal, message);
        }
        
        public static void Fatal(Exception ex, string message = "")
        {
            Error(ex, message);
            Log(LogLevel.Fatal, "Fatal error encountered, terminating...");
        }
        
        private static void Log(LogLevel level, string message)
        {
            if (level < _minLevel)
                return;
                
            var timestamp = _showTimestamps ? $"[{DateTime.Now:HH:mm:ss.fff}] " : "";
            var levelStr = level.ToString().ToUpper().PadRight(7);
            var fullMessage = $"{timestamp}{levelStr} {message}";
            
            // Choose output stream and color based on level
            var output = (level >= LogLevel.Error) ? Console.Error : Console.Out;
            var originalColor = Console.ForegroundColor;
            
            try
            {
                Console.ForegroundColor = level switch
                {
                    LogLevel.Debug => ConsoleColor.Gray,
                    LogLevel.Info => ConsoleColor.White,
                    LogLevel.Warning => ConsoleColor.Yellow,
                    LogLevel.Error => ConsoleColor.Red,
                    LogLevel.Fatal => ConsoleColor.Magenta,
                    _ => ConsoleColor.White
                };
                
                output.WriteLine(fullMessage);
            }
            finally
            {
                Console.ForegroundColor = originalColor;
            }
            
            // Also write to log file if configured
            _logFile?.WriteLine(fullMessage);
            _logFile?.Flush();
        }
        
        public static void Close()
        {
            _logFile?.Close();
            _logFile = null;
        }
    }
}