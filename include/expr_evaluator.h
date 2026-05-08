#pragma once

#include "type.h"
#include "stack.h"
#include "query_parser.h"
#include <cstdlib>
#include <cstring>

namespace nanodb {

// Expression evaluator: evaluates postfix expressions
class ExpressionEvaluator {
public:
    // Evaluate a postfix expression (for WHERE clauses)
    // context: optional context for variable lookups (not used in demo)
    static bool evaluate_boolean(const Token* postfix, int postfix_count) {
        Stack<Field> stack;
        
        for (int i = 0; i < postfix_count; ++i) {
            const Token& token = postfix[i];
            
            // Operands: push to stack
            if (token.type == TokenType::INT_LITERAL) {
                int val = atoi(token.value);
                stack.push(Field(new IntValue(val)));
                continue;
            }
            
            if (token.type == TokenType::FLOAT_LITERAL) {
                double val = atof(token.value);
                stack.push(Field(new FloatValue(val)));
                continue;
            }
            
            if (token.type == TokenType::STRING_LITERAL) {
                stack.push(Field(new StringValue(token.value)));
                continue;
            }
            
            // Operators: pop operands and apply
            if (token.type == TokenType::PLUS ||
                token.type == TokenType::MINUS ||
                token.type == TokenType::MULTIPLY ||
                token.type == TokenType::DIVIDE ||
                token.type == TokenType::MODULO ||
                token.type == TokenType::EQUAL ||
                token.type == TokenType::NOT_EQUAL ||
                token.type == TokenType::LESS ||
                token.type == TokenType::GREATER ||
                token.type == TokenType::LESS_EQUAL ||
                token.type == TokenType::GREATER_EQUAL ||
                token.type == TokenType::LOGICAL_AND ||
                token.type == TokenType::LOGICAL_OR) {
                
                if (stack.size() < 2) return false;
                
                Field b = stack.pop();
                Field a = stack.pop();
                
                Field result = apply_operator(a, b, token.type);
                stack.push(result);
            }
        }
        
        if (stack.size() != 1) return false;
        
        Field result = stack.pop();
        if (IntValue* iv = dynamic_cast<IntValue*>(result.val)) {
            return iv->v != 0;
        }
        return false;
    }
    
    // Apply a binary operator to two operands
    static Field apply_operator(const Field& a, const Field& b, TokenType op) {
        if (!a.val || !b.val) return Field(new IntValue(0));
        
        switch (op) {
            case TokenType::PLUS:
                if (auto av = dynamic_cast<IntValue*>(a.val)) {
                    if (auto bv = dynamic_cast<IntValue*>(b.val)) {
                        return Field(new IntValue(av->v + bv->v));
                    } else if (auto bv = dynamic_cast<FloatValue*>(b.val)) {
                        return Field(new FloatValue(av->v + bv->v));
                    }
                } else if (auto av = dynamic_cast<FloatValue*>(a.val)) {
                    if (auto bv = dynamic_cast<IntValue*>(b.val)) {
                        return Field(new FloatValue(av->v + bv->v));
                    } else if (auto bv = dynamic_cast<FloatValue*>(b.val)) {
                        return Field(new FloatValue(av->v + bv->v));
                    }
                }
                break;
                
            case TokenType::MINUS:
                if (auto av = dynamic_cast<IntValue*>(a.val)) {
                    if (auto bv = dynamic_cast<IntValue*>(b.val)) {
                        return Field(new IntValue(av->v - bv->v));
                    } else if (auto bv = dynamic_cast<FloatValue*>(b.val)) {
                        return Field(new FloatValue(av->v - bv->v));
                    }
                } else if (auto av = dynamic_cast<FloatValue*>(a.val)) {
                    if (auto bv = dynamic_cast<IntValue*>(b.val)) {
                        return Field(new FloatValue(av->v - bv->v));
                    } else if (auto bv = dynamic_cast<FloatValue*>(b.val)) {
                        return Field(new FloatValue(av->v - bv->v));
                    }
                }
                break;
                
            case TokenType::MULTIPLY:
                if (auto av = dynamic_cast<IntValue*>(a.val)) {
                    if (auto bv = dynamic_cast<IntValue*>(b.val)) {
                        return Field(new IntValue(av->v * bv->v));
                    } else if (auto bv = dynamic_cast<FloatValue*>(b.val)) {
                        return Field(new FloatValue(av->v * bv->v));
                    }
                } else if (auto av = dynamic_cast<FloatValue*>(a.val)) {
                    if (auto bv = dynamic_cast<IntValue*>(b.val)) {
                        return Field(new FloatValue(av->v * bv->v));
                    } else if (auto bv = dynamic_cast<FloatValue*>(b.val)) {
                        return Field(new FloatValue(av->v * bv->v));
                    }
                }
                break;
                
            case TokenType::DIVIDE:
                if (auto av = dynamic_cast<IntValue*>(a.val)) {
                    if (auto bv = dynamic_cast<IntValue*>(b.val)) {
                        if (bv->v != 0) return Field(new IntValue(av->v / bv->v));
                    }
                } else if (auto av = dynamic_cast<FloatValue*>(a.val)) {
                    if (auto bv = dynamic_cast<FloatValue*>(b.val)) {
                        if (bv->v != 0.0) return Field(new FloatValue(av->v / bv->v));
                    }
                }
                break;
                
            case TokenType::MODULO:
                if (auto av = dynamic_cast<IntValue*>(a.val)) {
                    if (auto bv = dynamic_cast<IntValue*>(b.val)) {
                        if (bv->v != 0) return Field(new IntValue(av->v % bv->v));
                    }
                }
                break;
                
            case TokenType::EQUAL:
                if (a.compare(b) == 0) return Field(new IntValue(1));
                return Field(new IntValue(0));
                
            case TokenType::NOT_EQUAL:
                if (a.compare(b) != 0) return Field(new IntValue(1));
                return Field(new IntValue(0));
                
            case TokenType::LESS:
                if (a.compare(b) < 0) return Field(new IntValue(1));
                return Field(new IntValue(0));
                
            case TokenType::GREATER:
                if (a.compare(b) > 0) return Field(new IntValue(1));
                return Field(new IntValue(0));
                
            case TokenType::LESS_EQUAL:
                if (a.compare(b) <= 0) return Field(new IntValue(1));
                return Field(new IntValue(0));
                
            case TokenType::GREATER_EQUAL:
                if (a.compare(b) >= 0) return Field(new IntValue(1));
                return Field(new IntValue(0));
                
            case TokenType::LOGICAL_AND:
                if ((a.val && a.compare(b) != 0) && (b.val && a.compare(b) != 0)) {
                    return Field(new IntValue(1));
                }
                return Field(new IntValue(0));
                
            case TokenType::LOGICAL_OR:
                if (a.val || b.val) return Field(new IntValue(1));
                return Field(new IntValue(0));
                
            default:
                break;
        }
        
        return Field(new IntValue(0));
    }
};

} // namespace nanodb
