#pragma once
// NanoDB Query Parser
// Tokenizes infix expressions and converts them to postfix via Shunting Yard.

#include "type.h"
#include "stack.h"
#include <string>
#include <cstring>
#include <cctype>

namespace nanodb {

// Token types for expression parsing
enum class TokenType {
    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    IDENTIFIER,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    MODULO,
    GREATER,
    LESS,
    EQUAL,
    NOT_EQUAL,
    GREATER_EQUAL,
    LESS_EQUAL,
    LOGICAL_AND,
    LOGICAL_OR,
    LEFT_PAREN,
    RIGHT_PAREN,
    END,
    UNKNOWN
};

struct Token {
    TokenType type;
    char value[256];
    
    Token() : type(TokenType::UNKNOWN) { value[0] = '\0'; }
    Token(TokenType t, const char* v = "") : type(t) {
        if (v) strncpy_s(value, sizeof(value), v, sizeof(value) - 1);
        else value[0] = '\0';
    }
};

// Query Parser: Converts infix expressions to postfix (Shunting Yard algorithm)
class QueryParser {
public:
    QueryParser() {}
    
    // Tokenize input expression
    bool tokenize(const char* expr, Token* tokens, int& token_count) {
        token_count = 0;
        int i = 0;
        int len = strlen(expr);
        
        while (i < len && token_count < 100) {
            // Skip whitespace
            while (i < len && isspace((unsigned char)expr[i])) ++i;
            if (i >= len) break;
            
            // Numbers
            if (isdigit((unsigned char)expr[i]) || (expr[i] == '.' && i+1 < len && isdigit((unsigned char)expr[i+1]))) {
                bool has_dot = false;
                int start = i;
                while (i < len && (isdigit((unsigned char)expr[i]) || (expr[i] == '.' && !has_dot))) {
                    if (expr[i] == '.') has_dot = true;
                    ++i;
                }
                int len_num = i - start;
                char num[256];
                strncpy_s(num, sizeof(num), expr + start, len_num);
                num[len_num] = '\0';
                tokens[token_count++] = Token(has_dot ? TokenType::FLOAT_LITERAL : TokenType::INT_LITERAL, num);
                continue;
            }
            
            // Strings
            if (expr[i] == '"') {
                ++i;
                int start = i;
                while (i < len && expr[i] != '"') ++i;
                int len_str = i - start;
                char str[256];
                strncpy_s(str, sizeof(str), expr + start, len_str);
                str[len_str] = '\0';
                tokens[token_count++] = Token(TokenType::STRING_LITERAL, str);
                if (i < len) ++i; // skip closing quote
                continue;
            }
            
            // Identifiers (column names)
            if (isalpha((unsigned char)expr[i]) || expr[i] == '_') {
                int start = i;
                while (i < len && (isalnum((unsigned char)expr[i]) || expr[i] == '_')) ++i;
                int len_id = i - start;
                char id[256];
                strncpy_s(id, sizeof(id), expr + start, len_id);
                id[len_id] = '\0';

                char upper_id[256];
                for (int k = 0; k <= len_id; ++k) {
                    upper_id[k] = (char)std::toupper((unsigned char)id[k]);
                }

                if (strcmp(upper_id, "AND") == 0) {
                    // Treat SQL-style boolean keywords as operators, not identifiers.
                    tokens[token_count++] = Token(TokenType::LOGICAL_AND, "AND");
                } else if (strcmp(upper_id, "OR") == 0) {
                    tokens[token_count++] = Token(TokenType::LOGICAL_OR, "OR");
                } else {
                    tokens[token_count++] = Token(TokenType::IDENTIFIER, id);
                }
                continue;
            }
            
            // Operators
            if (i + 1 < len) {
                if (expr[i] == '=' && expr[i+1] == '=') {
                    tokens[token_count++] = Token(TokenType::EQUAL, "==");
                    i += 2;
                    continue;
                }
                if (expr[i] == '!' && expr[i+1] == '=') {
                    tokens[token_count++] = Token(TokenType::NOT_EQUAL, "!=");
                    i += 2;
                    continue;
                }
                if (expr[i] == '<' && expr[i+1] == '=') {
                    tokens[token_count++] = Token(TokenType::LESS_EQUAL, "<=");
                    i += 2;
                    continue;
                }
                if (expr[i] == '>' && expr[i+1] == '=') {
                    tokens[token_count++] = Token(TokenType::GREATER_EQUAL, ">=");
                    i += 2;
                    continue;
                }
                if (expr[i] == '&' && expr[i+1] == '&') {
                    tokens[token_count++] = Token(TokenType::LOGICAL_AND, "&&");
                    i += 2;
                    continue;
                }
                if (expr[i] == '|' && expr[i+1] == '|') {
                    tokens[token_count++] = Token(TokenType::LOGICAL_OR, "||");
                    i += 2;
                    continue;
                }
            }
            
            // Single character operators
            switch (expr[i]) {
                case '+': tokens[token_count++] = Token(TokenType::PLUS, "+"); ++i; break;
                case '-': tokens[token_count++] = Token(TokenType::MINUS, "-"); ++i; break;
                case '*': tokens[token_count++] = Token(TokenType::MULTIPLY, "*"); ++i; break;
                case '/': tokens[token_count++] = Token(TokenType::DIVIDE, "/"); ++i; break;
                case '%': tokens[token_count++] = Token(TokenType::MODULO, "%"); ++i; break;
                case '<': tokens[token_count++] = Token(TokenType::LESS, "<"); ++i; break;
                case '>': tokens[token_count++] = Token(TokenType::GREATER, ">"); ++i; break;
                case '(': tokens[token_count++] = Token(TokenType::LEFT_PAREN, "("); ++i; break;
                case ')': tokens[token_count++] = Token(TokenType::RIGHT_PAREN, ")"); ++i; break;
                default: ++i; break;
            }
        }
        
        tokens[token_count] = Token(TokenType::END, "");
        return true;
    }
    
    // Get operator precedence
    static int get_precedence(TokenType type) {
        switch (type) {
            case TokenType::LOGICAL_OR: return 1;
            case TokenType::LOGICAL_AND: return 2;
            case TokenType::EQUAL:
            case TokenType::NOT_EQUAL:
            case TokenType::LESS:
            case TokenType::GREATER:
            case TokenType::LESS_EQUAL:
            case TokenType::GREATER_EQUAL: return 3;
            case TokenType::PLUS:
            case TokenType::MINUS: return 4;
            case TokenType::MULTIPLY:
            case TokenType::DIVIDE:
            case TokenType::MODULO: return 5;
            default: return 0;
        }
    }
    
    // Shunting yard algorithm: convert infix to postfix
    bool infix_to_postfix(const Token* tokens, int token_count, Token* postfix, int& postfix_count) {
        postfix_count = 0;
        Stack<Token> op_stack;
        
        for (int i = 0; i < token_count && tokens[i].type != TokenType::END; ++i) {
            const Token& token = tokens[i];
            
            // Operands: directly to output
            if (token.type == TokenType::INT_LITERAL ||
                token.type == TokenType::FLOAT_LITERAL ||
                token.type == TokenType::STRING_LITERAL ||
                token.type == TokenType::IDENTIFIER) {
                postfix[postfix_count++] = token;
                continue;
            }
            
            // Left paren: push to stack
            if (token.type == TokenType::LEFT_PAREN) {
                op_stack.push(token);
                continue;
            }
            
            // Right paren: pop until left paren
            if (token.type == TokenType::RIGHT_PAREN) {
                while (!op_stack.empty() && op_stack.top().type != TokenType::LEFT_PAREN) {
                    postfix[postfix_count++] = op_stack.pop();
                }
                if (!op_stack.empty()) op_stack.pop(); // pop left paren
                continue;
            }
            
            // Operators: pop higher precedence operators
                 // Left-associative operators are emitted while stack precedence is >= current.
            int prec = get_precedence(token.type);
            while (!op_stack.empty() && 
                   op_stack.top().type != TokenType::LEFT_PAREN &&
                   get_precedence(op_stack.top().type) >= prec) {
                postfix[postfix_count++] = op_stack.pop();
            }
            op_stack.push(token);
        }
        
        // Pop remaining operators
        while (!op_stack.empty()) {
            postfix[postfix_count++] = op_stack.pop();
        }
        
        return true;
    }
};

} // namespace nanodb
