#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <string>

#pragma once

const int MAX_IDENTIFIER_LENGTH = 1024; // Maximum length for identifiers

enum LableType {
    IF_LABEL,
    BLOCK_LABEL,
    WHILE_LABEL,
    FOR_LABEL,
};

class LabelAllocator {
public:
    std::string getLabel(LableType l) {
        if (l == IF_LABEL) {
            return std::to_string(ifLabelCounter++);
        } else if (l == BLOCK_LABEL) {
            return "BLOCK_" + std::to_string(blockLabelCounter++);
        } else if (l == WHILE_LABEL) {
            return std::to_string(whileLabelCounter++);
        } else if (l == FOR_LABEL) {
            return std::to_string(forLabelCounter++);
        }
    }
private:
    int ifLabelCounter = 0;
    int blockLabelCounter = 0;
    int whileLabelCounter = 0;
    int forLabelCounter = 0;
};
static LabelAllocator labelAllocator; // Static label allocator for generating unique labels


enum TokenType {
    T_EOF, T_PLUS, T_MINUS, T_STAR, T_SLASH,  T_LPAREN, T_RPAREN, T_LBRACE, T_RBRACE,
    T_IDENTIFIER, T_PRINT, T_IF, T_ELSE, T_WHILE, T_FOR,
    T_SEMI, T_NUMBER, T_INT, T_ASSIGN, T_COMMA,
    T_LT, T_GT, T_LE, T_GE, T_NE, T_EQ, T_NOT, T_AND, T_OR
};

enum PrimitiveType {
    P_INT, P_FLOAT, P_STRING, P_BOOL
};

enum StmtType {
    S_PRINT, S_ASSIGN, S_IF, S_WHILE, S_RETURN, S_BLOCK, S_EXPR, S_VARDEF, S_FOR
};

enum ExprType {
    A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE,
    A_EQ, A_NE, A_LT, A_LE, A_GT, A_GE, A_AND, A_OR
};

enum UnaryOp {
    U_PLUS, U_MINUS, U_NOT
};

struct Symbol {
    std::string name;
    PrimitiveType type;
    bool operator==(const std::string& other_name) const {
        return name == other_name;
    }
};

struct SymbolTable {
    std::vector<Symbol> symbols;

    // TODO: 作用域
    void addSymbol(std::string name, PrimitiveType type) {
        for (const auto& symbol : symbols) {
            if (symbol.name == name) {
                throw std::runtime_error("SymbolTable::addSymbol: Symbol already exists: " + name);
            }
        }
        symbols.push_back({name, type});
    }

    Symbol getSymbol(std::string name) {
        for (const auto& symbol : symbols) {
            if (symbol.name == name) {
                return symbol; // Return the found symbol
            }
        }
        throw std::runtime_error("SymbolTable::getSymbol: Symbol not found: " + name);
    }
};

static SymbolTable symbol_table;

struct Value {
    PrimitiveType type;
    union {
        int ivalue; // For integer literals
        double fvalue; // For floating-point literals, if needed
    };
    std::string strvalue; // For string literals, if needed
    void setIntValue(int value) {
        ivalue = value;
        type = P_INT;
    }
    void setFloatValue(double value) {
        fvalue = value;
        type = P_FLOAT;
    }
    void setStringValue(const std::string& value) {
        strvalue = value;
        type = P_STRING;
    }
    std::string toString() const {
        if (type == P_INT) {
            return std::to_string(ivalue);
        } else if (type == P_FLOAT) {
            return std::to_string(fvalue);
        }
        return "Unknown Value Type";
    }
    bool operator==(const Value& other) const {
        if (type != other.type) return false;
        if (type == P_INT) return ivalue == other.ivalue;
        if (type == P_FLOAT) return fvalue == other.fvalue;
        return false;
    }

    bool operator!=(const Value& other) const {
        return !(*this == other);
    }

    Value operator+(const Value& other) const {
        if (type == P_INT && other.type == P_INT) {
            return Value{P_INT, .ivalue = ivalue + other.ivalue};
        } else if (type == P_FLOAT && other.type == P_FLOAT) {
            return Value{P_FLOAT, .fvalue = fvalue + other.fvalue};
        } else if (type == P_INT && other.type == P_FLOAT) {
            return Value{P_FLOAT, .fvalue = ivalue + other.fvalue};
        } else if (type == P_FLOAT && other.type == P_INT) {
            return Value{P_FLOAT, .fvalue = fvalue + other.ivalue};
        }
        throw std::runtime_error("Value::operator+: Incompatible types for addition");
    }
    Value operator-(const Value& other) const {
        if (type == P_INT && other.type == P_INT) {
            return Value{P_INT, .ivalue = ivalue - other.ivalue};
        } else if (type == P_FLOAT && other.type == P_FLOAT) {
            return Value{P_FLOAT, .fvalue = fvalue - other.fvalue};
        } else if (type == P_INT && other.type == P_FLOAT) {
            return Value{P_FLOAT, .fvalue = ivalue - other.fvalue};
        } else if (type == P_FLOAT && other.type == P_INT) {
            return Value{P_FLOAT, .fvalue = fvalue - other.ivalue};
        }
        throw std::runtime_error("Value::operator-: Incompatible types for subtraction");
    }
    Value operator*(const Value& other) const {
        if (type == P_INT && other.type == P_INT) {
            return Value{P_INT, .ivalue = ivalue * other.ivalue};
        } else if (type == P_FLOAT && other.type == P_FLOAT) {
            return Value{P_FLOAT, .fvalue = fvalue * other.fvalue};
        } else if (type == P_INT && other.type == P_FLOAT) {
            return Value{P_FLOAT, .fvalue = ivalue * other.fvalue};
        } else if (type == P_FLOAT && other.type == P_INT) {
            return Value{P_FLOAT, .fvalue = fvalue * other.ivalue};
        }
        throw std::runtime_error("Value::operator*: Incompatible types for multiplication");
    }
    Value operator/(const Value& other) const {
        if (type == P_INT && other.type == P_INT) {
            if (other.ivalue == 0) throw std::runtime_error("Division by zero");
            return Value{P_INT, .ivalue = ivalue / other.ivalue};
        } else if (type == P_FLOAT && other.type == P_FLOAT) {
            if (other.fvalue == 0.0) throw std::runtime_error("Division by zero");
            return Value{P_FLOAT, .fvalue = fvalue / other.fvalue};
        } else if (type == P_INT && other.type == P_FLOAT) {
            if (other.fvalue == 0.0) throw std::runtime_error("Division by zero");
            return Value{P_FLOAT, .fvalue = ivalue / other.fvalue};
        } else if (type == P_FLOAT && other.type == P_INT) {
            if (other.ivalue == 0) throw std::runtime_error("Division by zero");
            return Value{P_FLOAT, .fvalue = fvalue / other.ivalue};
        }
        throw std::runtime_error("Value::operator/: Incompatible types for division");
    }
    Value operator-() const {
        if (type == P_INT) {
            return Value{P_INT, .ivalue = -ivalue};
        } else if (type == P_FLOAT) {
            return Value{P_FLOAT, .fvalue = -fvalue};
        }
        throw std::runtime_error("Value::operator-: Unknown value type for unary minus");
    }
};

struct Token {
    TokenType type;
    Value value; // Value of the token, if applicable
    int line_no; // Line number in the source file
    int column_no; // Column number in the source file
};


class ASTNode {
    public:
        ASTNode() = default;
        ~ASTNode() = default;
        virtual void walk(std::string prefix) = 0; // Pure virtual function for walking the AST
        virtual Value getValue() {
            throw std::runtime_error("ASTNode::getValue: Not implemented for this node type");
        };
        PrimitiveType getType() const {
            return value.type; // Return the type of the value
        }
        static std::string prettyPrint(std::string prefix) {
            if (prefix.empty()) {
                return "";
            }
            return prefix + "|--> ";
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

class StatementNode : public ASTNode {
    public:
        StatementNode() = default;
        ~StatementNode() = default;
        StmtType getStmtType() const {
            return type; // Return the type of the statement
        }
    protected:
        StmtType type; // Type of the statement
};

class ValueNode : public ExprNode {
    public:
        ValueNode(Value value) {this->value = value;}
        ~ValueNode() = default;
        void walk(std::string prefix) override {
            // Implement the walk method to print the value
            if (value.type == P_INT) {
                std::cout << prettyPrint(prefix) << "Integer Literal: " << value.ivalue << std::endl;
            } else if (value.type == P_FLOAT) {
                std::cout << prettyPrint(prefix) << "Float Literal: " << value.fvalue << std::endl;
            }
        }
        int getIntValue() const {
            if (value.type == P_INT) {
                return value.ivalue;
            } else if (value.type == P_FLOAT) {
                return static_cast<int>(value.fvalue); // Convert float to int for simplicity
            }
            throw std::runtime_error("ValueNode::getValue: Unknown value type");
        }
        double getFloatValue() const {
            if (value.type == P_FLOAT) {
                return value.fvalue;
            } else if (value.type == P_INT) {
                return static_cast<double>(value.ivalue); // Convert int to float for simplicity
            }
            throw std::runtime_error("ValueNode::getValue: Unknown value type");
        }
};



static const std::map<ExprType, int> precedence = {
    {A_ADD, 15},
    {A_SUBTRACT, 15},
    {A_MULTIPLY, 20},
    {A_DIVIDE, 20},
    {A_EQ, 5},
    {A_NE, 5},
    {A_LT, 10},
    {A_GT, 10},
    {A_LE, 10},
    {A_GE, 10},
    {A_AND, 3},
    {A_OR, 2}
};