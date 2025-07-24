#pragma once

#include <string>
#include <vector>
#include <rttr/type>
#include <rttr/instance.h>

namespace Haketon
{
    enum class TokenType
    {
        Identifier,
        NumberLiteral,
        StringLiteral,
        BoolLiteral,
        
        // Comparison operators
        Equal,        // ==
        NotEqual,     // !=
        Less,         // <
        Greater,      // >
        LessEqual,    // <=
        GreaterEqual, // >=
        
        // Logical operators
        And,          // &&
        Or,           // ||
        Not,          // !
        
        // Punctuation
        LeftParen,    // (
        RightParen,   // )
        
        EndOfInput,
        Invalid
    };
    
    struct Token
    {
        TokenType type;
        std::string value;
        size_t position;
    };
    
    class EditConditionEvaluator
    {
    public:
        static bool EvaluateCondition(const std::string& condition, rttr::instance& component);
        
    private:
        EditConditionEvaluator(const std::string& condition, rttr::instance& component);
        
        // Tokenization
        std::vector<Token> Tokenize();
        void SkipWhitespace();
        Token ReadIdentifier();
        Token ReadNumber();
        Token ReadStringLiteral();
        bool IsAtEnd() const { return m_Position >= m_Condition.length(); }
        char Peek() const { return IsAtEnd() ? '\0' : m_Condition[m_Position]; }
        char PeekNext() const { return m_Position + 1 >= m_Condition.length() ? '\0' : m_Condition[m_Position + 1]; }
        char Advance() { return IsAtEnd() ? '\0' : m_Condition[m_Position++]; }
        
        // Parsing (Recursive Descent)
        bool ParseExpression();
        bool ParseOrExpression();
        bool ParseAndExpression();
        bool ParseEqualityExpression();
        bool ParseComparisonExpression();
        bool ParseUnaryExpression();
        bool ParsePrimaryExpression();
        
        // Value operations
        rttr::variant GetPropertyValue(const std::string& propertyName);
        rttr::variant ParseLiteralValue(const Token& token);
        bool CompareValues(const rttr::variant& left, const rttr::variant& right, TokenType op);
        bool ConvertToBool(const rttr::variant& value);
        
        // Helper methods
        bool Match(TokenType type);
        Token Consume(TokenType type, const std::string& errorMessage);
        void Error(const std::string& message);
        
        std::string m_Condition;
        rttr::instance& m_Component;
        size_t m_Position;
        std::vector<Token> m_Tokens;
        size_t m_TokenIndex;
        std::string m_ErrorMessage;
    };
}