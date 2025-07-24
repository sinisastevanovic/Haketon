#include "EditConditionEvaluator.h"
#include "Haketon/Core/Log.h"
#include <cctype>
#include <sstream>

namespace Haketon
{
    bool EditConditionEvaluator::EvaluateCondition(const std::string& condition, rttr::instance& component)
    {
        try
        {
            EditConditionEvaluator evaluator(condition, component);
            return evaluator.ParseExpression();
        }
        catch (const std::exception& e)
        {
            HK_CORE_ERROR("EditCondition evaluation failed: {}", e.what());
            return true; // Default to enabled on error
        }
    }
    
    EditConditionEvaluator::EditConditionEvaluator(const std::string& condition, rttr::instance& component)
        : m_Condition(condition), m_Component(component), m_Position(0), m_TokenIndex(0)
    {
        m_Tokens = Tokenize();
    }
    
    std::vector<Token> EditConditionEvaluator::Tokenize()
    {
        std::vector<Token> tokens;
        m_Position = 0;
        
        while (!IsAtEnd())
        {
            SkipWhitespace();
            if (IsAtEnd()) break;
            
            size_t tokenStart = m_Position;
            char c = Peek();
            
            if (std::isalpha(c) || c == '_')
            {
                tokens.push_back(ReadIdentifier());
            }
            else if (std::isdigit(c) || c == '.')
            {
                tokens.push_back(ReadNumber());
            }
            else if (c == '\"' || c == '\'')
            {
                tokens.push_back(ReadStringLiteral());
            }
            else
            {
                Token token;
                token.position = tokenStart;
                
                switch (c)
                {
                    case '=':
                        if (PeekNext() == '=')
                        {
                            Advance(); Advance();
                            token.type = TokenType::Equal;
                            token.value = "==";
                        }
                        else
                        {
                            Error("Single '=' not supported, use '==' for comparison");
                        }
                        break;
                        
                    case '!':
                        if (PeekNext() == '=')
                        {
                            Advance(); Advance();
                            token.type = TokenType::NotEqual;
                            token.value = "!=";
                        }
                        else
                        {
                            Advance();
                            token.type = TokenType::Not;
                            token.value = "!";
                        }
                        break;
                        
                    case '<':
                        if (PeekNext() == '=')
                        {
                            Advance(); Advance();
                            token.type = TokenType::LessEqual;
                            token.value = "<=";
                        }
                        else
                        {
                            Advance();
                            token.type = TokenType::Less;
                            token.value = "<";
                        }
                        break;
                        
                    case '>':
                        if (PeekNext() == '=')
                        {
                            Advance(); Advance();
                            token.type = TokenType::GreaterEqual;
                            token.value = ">=";
                        }
                        else
                        {
                            Advance();
                            token.type = TokenType::Greater;
                            token.value = ">";
                        }
                        break;
                        
                    case '&':
                        if (PeekNext() == '&')
                        {
                            Advance(); Advance();
                            token.type = TokenType::And;
                            token.value = "&&";
                        }
                        else
                        {
                            Error("Single '&' not supported, use '&&' for logical AND");
                        }
                        break;
                        
                    case '|':
                        if (PeekNext() == '|')
                        {
                            Advance(); Advance();
                            token.type = TokenType::Or;
                            token.value = "||";
                        }
                        else
                        {
                            Error("Single '|' not supported, use '||' for logical OR");
                        }
                        break;
                        
                    case '(':
                        Advance();
                        token.type = TokenType::LeftParen;
                        token.value = "(";
                        break;
                        
                    case ')':
                        Advance();
                        token.type = TokenType::RightParen;
                        token.value = ")";
                        break;
                        
                    default:
                        Advance();
                        token.type = TokenType::Invalid;
                        token.value = c;
                        Error("Unexpected character: " + std::string(1, c));
                        break;
                }
                
                tokens.push_back(token);
            }
        }
        
        // Add end of input token
        Token endToken;
        endToken.type = TokenType::EndOfInput;
        endToken.position = m_Position;
        tokens.push_back(endToken);
        
        return tokens;
    }
    
    void EditConditionEvaluator::SkipWhitespace()
    {
        while (!IsAtEnd() && std::isspace(Peek()))
        {
            Advance();
        }
    }
    
    Token EditConditionEvaluator::ReadIdentifier()
    {
        Token token;
        token.position = m_Position;
        
        std::string value;
        while (!IsAtEnd() && (std::isalnum(Peek()) || Peek() == '_'))
        {
            value += Advance();
        }
        
        // Check for boolean literals
        if (value == "true")
        {
            token.type = TokenType::BoolLiteral;
        }
        else if (value == "false")
        {
            token.type = TokenType::BoolLiteral;
        }
        else
        {
            token.type = TokenType::Identifier;
        }
        
        token.value = value;
        return token;
    }
    
    Token EditConditionEvaluator::ReadNumber()
    {
        Token token;
        token.type = TokenType::NumberLiteral;
        token.position = m_Position;
        
        std::string value;
        bool hasDecimal = false;
        
        while (!IsAtEnd())
        {
            char c = Peek();
            if (std::isdigit(c))
            {
                value += Advance();
            }
            else if (c == '.' && !hasDecimal)
            {
                hasDecimal = true;
                value += Advance();
            }
            else if (c == 'f' || c == 'F')
            {
                value += Advance();
                break;
            }
            else
            {
                break;
            }
        }
        
        token.value = value;
        return token;
    }
    
    Token EditConditionEvaluator::ReadStringLiteral()
    {
        Token token;
        token.type = TokenType::StringLiteral;
        token.position = m_Position;
        
        char quote = Advance(); // Consume opening quote
        std::string value;
        
        while (!IsAtEnd() && Peek() != quote)
        {
            if (Peek() == '\\' && !IsAtEnd())
            {
                Advance(); // Skip backslash
                if (!IsAtEnd())
                {
                    char escaped = Advance();
                    switch (escaped)
                    {
                        case 'n': value += '\n'; break;
                        case 't': value += '\t'; break;
                        case 'r': value += '\r'; break;
                        case '\\': value += '\\'; break;
                        case '\"': value += '\"'; break;
                        case '\'': value += '\''; break;
                        default: value += escaped; break;
                    }
                }
            }
            else
            {
                value += Advance();
            }
        }
        
        if (!IsAtEnd())
        {
            Advance(); // Consume closing quote
        }
        else
        {
            Error("Unterminated string literal");
        }
        
        token.value = value;
        return token;
    }
    
    bool EditConditionEvaluator::ParseExpression()
    {
        return ParseOrExpression();
    }
    
    bool EditConditionEvaluator::ParseOrExpression()
    {
        bool result = ParseAndExpression();
        
        while (Match(TokenType::Or))
        {
            bool right = ParseAndExpression();
            result = result || right;
        }
        
        return result;
    }
    
    bool EditConditionEvaluator::ParseAndExpression()
    {
        bool result = ParseEqualityExpression();
        
        while (Match(TokenType::And))
        {
            bool right = ParseEqualityExpression();
            result = result && right;
        }
        
        return result;
    }
    
    bool EditConditionEvaluator::ParseEqualityExpression()
    {
        rttr::variant left = rttr::variant();
        bool isComparison = false;
        
        // Try to parse as comparison first
        if (m_TokenIndex < m_Tokens.size() && m_Tokens[m_TokenIndex].type == TokenType::Identifier)
        {
            // Look ahead to see if this is a comparison
            for (size_t i = m_TokenIndex + 1; i < m_Tokens.size(); i++)
            {
                TokenType type = m_Tokens[i].type;
                if (type == TokenType::Equal || type == TokenType::NotEqual)
                {
                    isComparison = true;
                    break;
                }
                else if (type != TokenType::Identifier && type != TokenType::NumberLiteral && 
                         type != TokenType::StringLiteral && type != TokenType::BoolLiteral)
                {
                    break;
                }
            }
        }
        
        if (isComparison)
        {
            // This is a comparison expression
            if (m_Tokens[m_TokenIndex].type == TokenType::Identifier)
            {
                std::string propertyName = m_Tokens[m_TokenIndex].value;
                m_TokenIndex++;
                left = GetPropertyValue(propertyName);
            }
            else
            {
                Error("Expected property name in comparison");
            }
            
            while (Match(TokenType::Equal) || Match(TokenType::NotEqual))
            {
                TokenType op = m_Tokens[m_TokenIndex - 1].type;
                rttr::variant right;
                
                if (m_Tokens[m_TokenIndex].type == TokenType::Identifier ||
                    m_Tokens[m_TokenIndex].type == TokenType::NumberLiteral ||
                    m_Tokens[m_TokenIndex].type == TokenType::StringLiteral ||
                    m_Tokens[m_TokenIndex].type == TokenType::BoolLiteral)
                {
                    right = ParseLiteralValue(m_Tokens[m_TokenIndex]);
                    m_TokenIndex++;
                }
                else
                {
                    Error("Expected value in comparison");
                }
                
                return CompareValues(left, right, op);
            }
        }
        
        // Fall back to parsing as other expressions
        return ParseComparisonExpression();
    }
    
    bool EditConditionEvaluator::ParseComparisonExpression()
    {
        // For boolean expressions, delegate to unary
        if (m_TokenIndex < m_Tokens.size() && 
            (m_Tokens[m_TokenIndex].type == TokenType::Not || 
             m_Tokens[m_TokenIndex].type == TokenType::Identifier ||
             m_Tokens[m_TokenIndex].type == TokenType::BoolLiteral ||
             m_Tokens[m_TokenIndex].type == TokenType::LeftParen))
        {
            // Check if this is actually a numeric comparison
            bool isNumericComparison = false;
            if (m_Tokens[m_TokenIndex].type == TokenType::Identifier)
            {
                for (size_t i = m_TokenIndex + 1; i < m_Tokens.size(); i++)
                {
                    TokenType type = m_Tokens[i].type;
                    if (type == TokenType::Less || type == TokenType::Greater || 
                        type == TokenType::LessEqual || type == TokenType::GreaterEqual)
                    {
                        isNumericComparison = true;
                        break;
                    }
                    else if (type != TokenType::Identifier && type != TokenType::NumberLiteral)
                    {
                        break;
                    }
                }
            }
            
            if (isNumericComparison)
            {
                // Parse numeric comparison
                std::string propertyName = m_Tokens[m_TokenIndex].value;
                m_TokenIndex++;
                rttr::variant left = GetPropertyValue(propertyName);
                
                if (Match(TokenType::Less) || Match(TokenType::Greater) || 
                    Match(TokenType::LessEqual) || Match(TokenType::GreaterEqual))
                {
                    TokenType op = m_Tokens[m_TokenIndex - 1].type;
                    rttr::variant right = ParseLiteralValue(m_Tokens[m_TokenIndex]);
                    m_TokenIndex++;
                    
                    return CompareValues(left, right, op);
                }
            }
        }
        
        return ParseUnaryExpression();
    }
    
    bool EditConditionEvaluator::ParseUnaryExpression()
    {
        if (Match(TokenType::Not))
        {
            return !ParseUnaryExpression();
        }
        
        return ParsePrimaryExpression();
    }
    
    bool EditConditionEvaluator::ParsePrimaryExpression()
    {
        if (Match(TokenType::LeftParen))
        {
            bool result = ParseExpression();
            Consume(TokenType::RightParen, "Expected ')' after expression");
            return result;
        }
        
        if (m_Tokens[m_TokenIndex].type == TokenType::Identifier)
        {
            std::string propertyName = m_Tokens[m_TokenIndex].value;
            m_TokenIndex++;
            rttr::variant value = GetPropertyValue(propertyName);
            return ConvertToBool(value);
        }
        
        if (m_Tokens[m_TokenIndex].type == TokenType::BoolLiteral)
        {
            bool result = m_Tokens[m_TokenIndex].value == "true";
            m_TokenIndex++;
            return result;
        }
        
        Error("Expected identifier, boolean literal, or '(' in expression");
        return false;
    }
    
    rttr::variant EditConditionEvaluator::GetPropertyValue(const std::string& propertyName)
    {
        auto property = m_Component.get_type().get_property(propertyName);
        if (!property.is_valid())
        {
            Error("Property '" + propertyName + "' not found");
        }
        
        return property.get_value(m_Component);
    }
    
    rttr::variant EditConditionEvaluator::ParseLiteralValue(const Token& token)
    {
        switch (token.type)
        {
            case TokenType::NumberLiteral:
            {
                std::string value = token.value;
                if (value.find('.') != std::string::npos || value.back() == 'f' || value.back() == 'F')
                {
                    return std::stof(value);
                }
                else
                {
                    return std::stoi(value);
                }
            }
            
            case TokenType::StringLiteral:
                return token.value;
                
            case TokenType::BoolLiteral:
                return token.value == "true";
                
            case TokenType::Identifier:
                // This might be an enum value - try to parse it
                return token.value;
                
            default:
                Error("Invalid literal type");
                return rttr::variant();
        }
    }
    
    bool EditConditionEvaluator::CompareValues(const rttr::variant& left, const rttr::variant& right, TokenType op)
    {
        // Handle enum comparisons
        if (left.get_type().is_enumeration())
        {
            std::string leftStr = left.to_string();
            std::string rightStr;
            
            if (right.get_type() == rttr::type::get<std::string>())
            {
                rightStr = right.get_value<std::string>();
            }
            else
            {
                rightStr = right.to_string();
            }
            
            switch (op)
            {
                case TokenType::Equal: return leftStr == rightStr;
                case TokenType::NotEqual: return leftStr != rightStr;
                default:
                    Error("Invalid comparison operator for enum type");
                    return false;
            }
        }
        
        // Handle arithmetic comparisons
        if (left.get_type().is_arithmetic() && 
            (right.get_type().is_arithmetic() || right.get_type() == rttr::type::get<std::string>()))
        {
            double leftVal = 0.0;
            double rightVal = 0.0;
            
            // Convert left operand
            if (left.get_type() == rttr::type::get<bool>())
                leftVal = left.get_value<bool>() ? 1.0 : 0.0;
            else if (left.get_type() == rttr::type::get<int>())
                leftVal = static_cast<double>(left.get_value<int>());
            else if (left.get_type() == rttr::type::get<float>())
                leftVal = static_cast<double>(left.get_value<float>());
            else if (left.get_type() == rttr::type::get<double>())
                leftVal = left.get_value<double>();
            else
                leftVal = left.to_double();
            
            // Convert right operand
            if (right.get_type() == rttr::type::get<int>())
                rightVal = static_cast<double>(right.get_value<int>());
            else if (right.get_type() == rttr::type::get<float>())
                rightVal = static_cast<double>(right.get_value<float>());
            else if (right.get_type() == rttr::type::get<double>())
                rightVal = right.get_value<double>();
            else
                rightVal = right.to_double();
            
            switch (op)
            {
                case TokenType::Equal: return std::abs(leftVal - rightVal) < 1e-9;
                case TokenType::NotEqual: return std::abs(leftVal - rightVal) >= 1e-9;
                case TokenType::Less: return leftVal < rightVal;
                case TokenType::Greater: return leftVal > rightVal;
                case TokenType::LessEqual: return leftVal <= rightVal;
                case TokenType::GreaterEqual: return leftVal >= rightVal;
                default:
                    Error("Invalid comparison operator");
                    return false;
            }
        }
        
        // Handle string comparisons
        if (left.get_type() == rttr::type::get<std::string>() || 
            right.get_type() == rttr::type::get<std::string>())
        {
            std::string leftStr = left.to_string();
            std::string rightStr = right.to_string();
            
            switch (op)
            {
                case TokenType::Equal: return leftStr == rightStr;
                case TokenType::NotEqual: return leftStr != rightStr;
                default:
                    Error("Only == and != supported for string comparisons");
                    return false;
            }
        }
        
        Error("Unsupported types for comparison");
        return false;
    }
    
    bool EditConditionEvaluator::ConvertToBool(const rttr::variant& value)
    {
        if (value.get_type() == rttr::type::get<bool>())
        {
            return value.get_value<bool>();
        }
        else if (value.get_type().is_arithmetic())
        {
            return value.to_double() != 0.0;
        }
        else if (value.get_type() == rttr::type::get<std::string>())
        {
            std::string str = value.get_value<std::string>();
            return !str.empty() && str != "0" && str != "false";
        }
        
        // For other types, consider them true if they exist
        return value.is_valid();
    }
    
    bool EditConditionEvaluator::Match(TokenType type)
    {
        if (m_TokenIndex >= m_Tokens.size() || m_Tokens[m_TokenIndex].type != type)
        {
            return false;
        }
        
        m_TokenIndex++;
        return true;
    }
    
    Token EditConditionEvaluator::Consume(TokenType type, const std::string& errorMessage)
    {
        if (m_TokenIndex >= m_Tokens.size() || m_Tokens[m_TokenIndex].type != type)
        {
            Error(errorMessage);
        }
        
        return m_Tokens[m_TokenIndex++];
    }
    
    void EditConditionEvaluator::Error(const std::string& message)
    {
        std::stringstream ss;
        ss << "EditCondition parse error";
        if (m_TokenIndex < m_Tokens.size())
        {
            ss << " at position " << m_Tokens[m_TokenIndex].position;
        }
        ss << ": " << message;
        
        throw std::runtime_error(ss.str());
    }
}