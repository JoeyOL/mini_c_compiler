#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <cassert>

#pragma once

const int MAX_IDENTIFIER_LENGTH = 1024; // Maximum length for identifiers



enum LableType {
    IF_LABEL,
    BLOCK_LABEL,
    WHILE_LABEL,
    FOR_LABEL,
    FUNCT_LABEL,
    FLOAT_CONSTANT_LABEL // Label type for float constants
};

extern std::map<double, std::string> float_constants; // Map to store float constants for unique representation

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
        } else if (l == FUNCT_LABEL) {
            return "FUNCT_END" + std::to_string(functLabelCounter++);
        } else if (l == FLOAT_CONSTANT_LABEL) {
            // Generate a unique label for float constants
            return "FLOAT_CONST_" + std::to_string(floatConstantCounter++);
        } else {
            throw std::runtime_error("LabelAllocator::getLabel: Unknown label type");
        }
    }
private:
    int ifLabelCounter = 0;
    int blockLabelCounter = 0;
    int functLabelCounter = 0; // Counter for function labels
    int whileLabelCounter = 0;
    int forLabelCounter = 0;
    int floatConstantCounter = 0;
};
static LabelAllocator labelAllocator; // Static label allocator for generating unique labels


enum TokenType {
    T_EOF, T_PLUS, T_MINUS, T_STAR, T_SLASH,  T_LPAREN, T_RPAREN, T_LBRACE, T_RBRACE,
    T_IDENTIFIER, T_PRINT, T_IF, T_ELSE, T_WHILE, T_FOR, T_RETURN,
    T_SEMI, T_NUMBER, T_INT, T_ASSIGN, T_COMMA, T_VOID, T_CHAR, T_FLOAT, T_LONG,
    T_LT, T_GT, T_LE, T_GE, T_NE, T_EQ, T_NOT, T_LOGAND, T_LOGOR, T_AMPER, T_AND, T_OR
};

enum PrimitiveType {
    P_NONE,
    P_INT, P_FLOAT, P_VOID, P_STRING, P_CHAR, P_LONG,
    P_INTPTR, P_FLOATPTR, P_CHARPTR, P_LONGPTR, P_VOIDPTR
};

inline bool is_pointer(PrimitiveType type) {
    // Check if the type is a pointer type
    return type == P_CHARPTR || type == P_FLOATPTR || type == P_INTPTR || type == P_LONGPTR || type == P_VOIDPTR;
}

enum StmtType {
    S_PRINT, S_ASSIGN, S_IF, S_WHILE, S_RETURN, S_BLOCK, S_EXPR, S_VARDEF, S_FOR, S_FUNCTDEF,
    S_FUNCTCALL
};

enum ExprType {
    A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE,
    A_EQ, A_NE, A_LT, A_LE, A_GT, A_GE, A_AND, A_OR, A_ASSIGN
};

enum UnaryOp {
    U_PLUS, U_MINUS, U_NOT,
    U_ADDR, U_DEREF, U_TRANSFORM, U_SCALE
};

struct Symbol {
    std::string name;
    PrimitiveType type;
    int size;
    bool operator==(const std::string& other_name) const {
        return name == other_name;
    }
};

struct Function {
    std::string name;
    PrimitiveType return_type;
    bool has_return;
};

struct SymbolTable {
    std::vector<Symbol> symbols;
    std::vector<Function> functions;
    Function current_function; // Current function being processed
    SymbolTable() {
        addFunction("printint", P_VOID); // Add built-in function for printing integers
        addFunction("printfloat", P_VOID); // Add built-in function for printing floats
        addFunction("printchar", P_VOID); // Add built-in function for printing characters
        addFunction("printlong", P_VOID); // Add built-in function for printing long integers
    };

    int typeToSize(PrimitiveType type) const {
        switch (type) {
            case P_INT: return 4;
            case P_FLOAT: return 8;
            case P_CHAR: return 1;
            case P_LONG: return 8;
            case P_VOIDPTR : case P_INTPTR: case P_FLOATPTR: case P_CHARPTR: case P_LONGPTR:
                return 8; // Pointer size is typically 8 bytes on modern architectures
            default: throw std::runtime_error("SymbolTable::typeToSize: Unknown type");
        }
    }
    // TODO: 作用域
    void addSymbol(std::string name, PrimitiveType type) {
        for (const auto& symbol : symbols) {
            if (symbol.name == name) {
                throw std::runtime_error("SymbolTable::addSymbol: Symbol already exists: " + name);
            }
        }
        symbols.push_back({name, type, typeToSize(type)});
    }

    Symbol getSymbol(std::string name) {
        for (const auto& symbol : symbols) {
            if (symbol.name == name) {
                return symbol; // Return the found symbol
            }
        }
        throw std::runtime_error("SymbolTable::getSymbol: Symbol not found: " + name);
    }

    void addFunction(std::string name, PrimitiveType return_type) {
        for (const auto& func : functions) {
            if (func.name == name) {
                throw std::runtime_error("SymbolTable::addFunction: Function already exists: " + name);
            }
        }
        functions.push_back({name, return_type, false});
    }
    Function getFunction(std::string name) {
        for (const auto& func : functions) {
            if (func.name == name) {
                return func; // Return the found function
            }
        }
        throw std::runtime_error("SymbolTable::getFunction: Function not found: " + name);
    }
    void setCurrentFunction(const Function& func) {
        current_function = func; // Set the current function being processed
    }
    Function getCurrentFunction() const {
        return current_function; // Get the current function being processed
    }
    void setCurrentFunctionReturn(bool has_return) {
        current_function.has_return = has_return; // Set whether the current function has a return statement
    }
};

extern SymbolTable symbol_table;

struct Value {
    PrimitiveType type;
    union {
        int ivalue; // For integer literals
        long lvalue; // For long integer literals, if needed
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
    void setLongValue(long value) {
        lvalue = value;
        type = P_LONG;
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

struct Reg {
    PrimitiveType type;
    int idx;
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
        // virtual Value getValue() {
        //     throw std::runtime_error("ASTNode::getValue: Not implemented for this node type");
        // };

        // 获取计算类型
        virtual PrimitiveType getPrimitiveType() const {
            return type; // Return the type of the value
        }
        virtual PrimitiveType getCalculateType() const {
            if (is_pointer(type)) {
                return P_LONG;
            }
            return type; // Return the calculated type of the AST node
        }
        static std::string prettyPrint(std::string prefix) {
            if (prefix.empty()) {
                return "";
            }
            return prefix + "|--> ";
        }
        virtual bool& isNeedTransform() {
            return need_transform; // Return whether the node needs transformation
        }
    protected:
        PrimitiveType type; // Type of the AST node
        bool need_transform = false; // Flag to indicate if the node needs transformation

};

class StatementNode : public ASTNode {
    public:
        StatementNode() = default;
        ~StatementNode() = default;
        StmtType getStmtType() const {
            return stmt_type; // Return the type of the statement
        }
    protected:
        StmtType stmt_type; // Type of the statement
};

class ExprNode: public StatementNode {
    public:
        ExprNode() {
            stmt_type = S_EXPR;
            offset = -1; 
        };
        ~ExprNode() = default;
        // virtual ExprType getType() const = 0; // Pure virtual function to get the expression type
        // virtual std::string convertTypeToString() const = 0; // Convert expression type to string
        Value getValue() {
            return value; // Return the value of the expression node
        }
        void setOffset(int offset) {
            this->offset = offset; // Set the offset for the expression node
        }
        int getOffset() const {
            if (offset != -1) {
                return offset; // If offset is set, return it
            }
            if (type_to_offset.find(this->type) == type_to_offset.end()) {
                return 0; // If the type is not found in the map, return 0
            }
            return type_to_offset.at(this->type); // Get the offset for the expression node
        }
        void setLValue(bool lvalue) {
            this->lvalue = lvalue; // Set whether the expression node is an lvalue
        }

        bool isLValue() const {
            return lvalue; // Return whether the expression node is an lvalue
        }
    protected:
        Value value; 
        int offset;
        std::map<PrimitiveType, int> type_to_offset = {
            {P_INT, 1},
            {P_FLOAT, 1},
            {P_CHAR, 1},
            {P_LONG, 1},
            {P_INTPTR, 4},
            {P_FLOATPTR, 8},
            {P_CHARPTR, 1},
            {P_LONGPTR, 8},
            {P_VOIDPTR, 0}
        };
        bool lvalue = false; // Flag to indicate if the expression is an lvalue
};



class ValueNode : public ExprNode {
    public:
        ValueNode(Value value) {
            this->value = value;
            type = value.type; // Set the type based on the value
        }
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

// 目前只支持一层指针
inline PrimitiveType pointTo(const PrimitiveType &type) {
    if (type == P_INT) {
        return P_INTPTR; // Return pointer type for int
    } else if (type == P_FLOAT) {
        return P_FLOATPTR; // Return pointer type for float
    } else if (type == P_CHAR) {
        return P_CHARPTR; // Return pointer type for char
    } else if (type == P_LONG) {
        return P_LONGPTR;
    } else if (type == P_VOID) {
        return P_VOIDPTR; // Return pointer type for void
    }
    throw std::runtime_error("VariableDeclareNode: Unknown type for pointer declaration");
}

// 获取指针指向的类型
inline PrimitiveType valueAt(const PrimitiveType &type) {
    switch (type) {
        case P_INTPTR: return P_INT; // Return int for int pointer
        case P_FLOATPTR: return P_FLOAT; // Return float for float pointer
        case P_CHARPTR: return P_CHAR; // Return char for char pointer
        case P_LONGPTR: return P_LONG;
        case P_VOIDPTR: return P_VOID; // Return void for void pointer
        default: throw std::runtime_error("VariableDeclareNode: Unknown type for value at pointer declaration");
    }
}