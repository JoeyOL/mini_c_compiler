#include <map>
#include <string>
#include <iostream>
#pragma once

enum TokenType {
    T_PLUS, T_MINUS, T_STAR, T_SLASH, T_INTLIT, T_LPAREN, T_RPAREN, T_FLOATLIT
};

struct Value {
    TokenType type;
    union {
        int ivalue; // For integer literals
        double fvalue; // For floating-point literals, if needed
    };
    void setIntValue(int value) {
        ivalue = value;
        type = T_INTLIT;
    }
    void setFloatValue(double value) {
        fvalue = value;
        type = T_FLOATLIT;
    }
    std::string toString() const {
        if (type == T_INTLIT) {
            return std::to_string(ivalue);
        } else if (type == T_FLOATLIT) {
            return std::to_string(fvalue);
        }
        return "Unknown Value Type";
    }
    bool operator==(const Value& other) const {
        if (type != other.type) return false;
        if (type == T_INTLIT) return ivalue == other.ivalue;
        if (type == T_FLOATLIT) return fvalue == other.fvalue;
        return false;
    }

    bool operator!=(const Value& other) const {
        return !(*this == other);
    }

    Value operator+(const Value& other) const {
        if (type == T_INTLIT && other.type == T_INTLIT) {
            return Value{T_INTLIT, .ivalue = ivalue + other.ivalue};
        } else if (type == T_FLOATLIT && other.type == T_FLOATLIT) {
            return Value{T_FLOATLIT, .fvalue = fvalue + other.fvalue};
        } else if (type == T_INTLIT && other.type == T_FLOATLIT) {
            return Value{T_FLOATLIT, .fvalue = ivalue + other.fvalue};
        } else if (type == T_FLOATLIT && other.type == T_INTLIT) {
            return Value{T_FLOATLIT, .fvalue = fvalue + other.ivalue};
        }
        throw std::runtime_error("Value::operator+: Incompatible types for addition");
    }
    Value operator-(const Value& other) const {
        if (type == T_INTLIT && other.type == T_INTLIT) {
            return Value{T_INTLIT, .ivalue = ivalue - other.ivalue};
        } else if (type == T_FLOATLIT && other.type == T_FLOATLIT) {
            return Value{T_FLOATLIT, .fvalue = fvalue - other.fvalue};
        } else if (type == T_INTLIT && other.type == T_FLOATLIT) {
            return Value{T_FLOATLIT, .fvalue = ivalue - other.fvalue};
        } else if (type == T_FLOATLIT && other.type == T_INTLIT) {
            return Value{T_FLOATLIT, .fvalue = fvalue - other.ivalue};
        }
        throw std::runtime_error("Value::operator-: Incompatible types for subtraction");
    }
    Value operator*(const Value& other) const {
        if (type == T_INTLIT && other.type == T_INTLIT) {
            return Value{T_INTLIT, .ivalue = ivalue * other.ivalue};
        } else if (type == T_FLOATLIT && other.type == T_FLOATLIT) {
            return Value{T_FLOATLIT, .fvalue = fvalue * other.fvalue};
        } else if (type == T_INTLIT && other.type == T_FLOATLIT) {
            return Value{T_FLOATLIT, .fvalue = ivalue * other.fvalue};
        } else if (type == T_FLOATLIT && other.type == T_INTLIT) {
            return Value{T_FLOATLIT, .fvalue = fvalue * other.ivalue};
        }
        throw std::runtime_error("Value::operator*: Incompatible types for multiplication");
    }
    Value operator/(const Value& other) const {
        if (type == T_INTLIT && other.type == T_INTLIT) {
            if (other.ivalue == 0) throw std::runtime_error("Division by zero");
            return Value{T_INTLIT, .ivalue = ivalue / other.ivalue};
        } else if (type == T_FLOATLIT && other.type == T_FLOATLIT) {
            if (other.fvalue == 0.0) throw std::runtime_error("Division by zero");
            return Value{T_FLOATLIT, .fvalue = fvalue / other.fvalue};
        } else if (type == T_INTLIT && other.type == T_FLOATLIT) {
            if (other.fvalue == 0.0) throw std::runtime_error("Division by zero");
            return Value{T_FLOATLIT, .fvalue = ivalue / other.fvalue};
        } else if (type == T_FLOATLIT && other.type == T_INTLIT) {
            if (other.ivalue == 0) throw std::runtime_error("Division by zero");
            return Value{T_FLOATLIT, .fvalue = fvalue / other.ivalue};
        }
        throw std::runtime_error("Value::operator/: Incompatible types for division");
    }
    Value operator-() const {
        if (type == T_INTLIT) {
            return Value{T_INTLIT, .ivalue = -ivalue};
        } else if (type == T_FLOATLIT) {
            return Value{T_FLOATLIT, .fvalue = -fvalue};
        }
        throw std::runtime_error("Value::operator-: Unknown value type for unary minus");
    }
};

struct Token {
    int type;
    Value value; // Value of the token, if applicable
    int line_no; // Line number in the source file
    int column_no; // Column number in the source file
};

enum ExprType {
    A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE
};

enum UnaryOp {
    U_PLUS, U_MINUS
};

class ASTNode {
    public:
        ASTNode() = default;
        ~ASTNode() = default;
        virtual void walk() = 0; // Pure virtual function for walking the AST
        virtual Value getValue() = 0;
        TokenType getType() const {
            return value.type; // Return the type of the value
        }
    protected:
        Value value; 
};

class ExprNode: public ASTNode {
    public:
        ExprNode() = default;
        ~ExprNode() = default;
        // virtual ExprType getType() const = 0; // Pure virtual function to get the expression type
        // virtual std::string convertTypeToString() const = 0; // Convert expression type to string
        Value getValue() override {
            return value; // Return the value of the expression node
        }
};

class ValueNode : public ExprNode{
    public:
        ValueNode(Value value) {this->value = value;}
        ~ValueNode() = default;
        void walk() override {
            // Implement the walk method to print the value
            if (value.type == T_INTLIT) {
                std::cout << "Integer Literal: " << value.ivalue << std::endl;
            } else if (value.type == T_FLOATLIT) {
                std::cout << "Floating Point Literal: " << value.fvalue << std::endl;
            }
        }
        int getIntValue() const {
            if (value.type == T_INTLIT) {
                return value.ivalue;
            } else if (value.type == T_FLOATLIT) {
                return static_cast<int>(value.fvalue); // Convert float to int for simplicity
            }
            throw std::runtime_error("ValueNode::getValue: Unknown value type");
        }
        double getFloatValue() const {
            if (value.type == T_FLOATLIT) {
                return value.fvalue;
            } else if (value.type == T_INTLIT) {
                return static_cast<double>(value.ivalue); // Convert int to float for simplicity
            }
            throw std::runtime_error("ValueNode::getValue: Unknown value type");
        }


};



static const std::map<ExprType, int> precedence = {
    {A_ADD, 1},
    {A_SUBTRACT, 1},
    {A_MULTIPLY, 2},
    {A_DIVIDE, 2},
};