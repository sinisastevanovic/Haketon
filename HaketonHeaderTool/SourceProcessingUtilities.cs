using System;
using System.Text;

namespace HaketonHeaderTool
{
    public static class SourceProcessingUtilities
    {
        public static string RemoveComments(string source)
        {
            if (string.IsNullOrEmpty(source))
                return source;
                
            var result = new StringBuilder(source.Length);
            int i = 0;
            
            while (i < source.Length)
            {
                char c = source[i];
                
                // Handle string literals (both " and ')
                if (c == '"' || c == '\'')
                {
                    char quote = c;
                    result.Append(c);
                    i++;
                    
                    // Copy everything inside the string literal, handling escape sequences
                    while (i < source.Length)
                    {
                        char stringChar = source[i];
                        result.Append(stringChar);
                        
                        if (stringChar == '\\' && i + 1 < source.Length)
                        {
                            // Skip escaped character
                            i++;
                            result.Append(source[i]);
                        }
                        else if (stringChar == quote)
                        {
                            // End of string literal
                            break;
                        }
                        i++;
                    }
                    i++;
                    continue;
                }
                
                // Handle single-line comments //
                if (c == '/' && i + 1 < source.Length && source[i + 1] == '/')
                {
                    // Skip until end of line
                    i += 2;
                    while (i < source.Length && source[i] != '\r' && source[i] != '\n')
                    {
                        i++;
                    }
                    // Keep the newline character(s)
                    if (i < source.Length && source[i] == '\r')
                    {
                        result.Append('\r');
                        i++;
                    }
                    if (i < source.Length && source[i] == '\n')
                    {
                        result.Append('\n');
                        i++;
                    }
                    continue;
                }
                
                // Handle multi-line comments /* */
                if (c == '/' && i + 1 < source.Length && source[i + 1] == '*')
                {
                    // Skip until */
                    i += 2;
                    while (i + 1 < source.Length)
                    {
                        if (source[i] == '*' && source[i + 1] == '/')
                        {
                            i += 2;
                            break;
                        }
                        // Preserve newlines inside comments to maintain line numbering
                        if (source[i] == '\r' || source[i] == '\n')
                        {
                            result.Append(source[i]);
                        }
                        i++;
                    }
                    continue;
                }
                
                // Regular character
                result.Append(c);
                i++;
            }
            
            return result.ToString();
        }
    }
}