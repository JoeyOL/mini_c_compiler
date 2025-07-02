#include "common/defs.h"
#include <memory>  
#include <iostream>
#include <map>
#pragma once

// 考虑指针加减法的offset问题，首先最终一定是先遍历到一个指针叶节点，然后往传递这个信息，直到遇到解引用操作，会更新offset，
// 那么下面实现一种方法，在每一个ExprNode中添加一个offset字段，表示当前节点的偏移量，然后在进行加减法时，需要乘上这个offset
// 这个操作称之为对齐，A_SCALE，并且限制在一个Expr中，只允许ptr和int、char、long等类型进行加减法操作，
// 这样可以避免指针和指针之间的加减法操作，以及指针和float之间的加减法操作，

class BinaryExpNode : public ExprNode {
    public:
        BinaryExpNode(ExprType op, std::shared_ptr<ExprNode> left, std::shared_ptr<ExprNode> right)
            : op(op), left(std::move(left)), right(std::move(right)) {
                type = P_NONE;
            }

        ExprType getOp() const { return op; }
        std::shared_ptr<ExprNode> getLeft() const { return left; }
        std::shared_ptr<ExprNode> getRight() const { return right; }
        std::string convertTypeToString() const {
            static const std::map<ExprType, std::string> typeToString = {
                {A_ADD, "Addition"},
                {A_SUBTRACT, "Subtraction"},
                {A_MULTIPLY, "Multiplication"},
                {A_DIVIDE, "Division"},
                {A_EQ, "Equality"},
                {A_NE, "Inequality"},
                {A_LT, "Less Than"},
                {A_LE, "Less Than or Equal"},
                {A_GT, "Greater Than"},
                {A_GE, "Greater Than or Equal"},
                {A_AND, "And"},
                {A_OR, "Or"},
                {A_ASSIGN, "Assignment"},
                {A_XOR, "XOR"},
                {A_LSHIFT, "Left Shift"},
                {A_RSHIFT, "Right Shift"},

            };
            auto it = typeToString.find(op);
            if (it != typeToString.end()) {
                return it->second;
            } else {
                return "Unknown Type";
            }
        }
        void walk(std::string prefix) override {
            // Implement the walk method to traverse the binary expression node
            // This could involve visiting the left and right nodes, etc.
            std::cout << prettyPrint(prefix) << "Binary Expr, Op "<< convertTypeToString() <<" :" << std::endl;
            left->walk(prefix + "\t");
            right->walk(prefix + "\t");
        }

        void updateCalType() {

            PrimitiveType left_type = left->getCalculateType();
            PrimitiveType right_type = right->getCalculateType();

            if (left_type== P_FLOAT || right_type == P_FLOAT) {
                cal_type = P_FLOAT;
            } else if (left_type == P_LONG || right_type == P_LONG) {
                cal_type = P_LONG; // If either is long, the result is long
            } else if (left_type == P_INT || right_type == P_INT) {
                cal_type = P_INT;
                if (op == A_GE || op == A_GT || op == A_LE || op == A_LT || op == A_EQ || op == A_NE) {
                    cal_type = P_LONG; // Comparison operations return an integer type
                    return;
                }
            } else if (left_type == P_CHAR || right_type == P_CHAR) {
                cal_type = P_CHAR; // If either is char, the result is char
                if (op == A_GE || op == A_GT || op == A_LE || op == A_LT || op == A_EQ || op == A_NE) {
                    cal_type = P_LONG; // Comparison operations return an integer type
                    return;
                }
            } 
        }

        void updateTypeAfterCal() {
            if (op == A_GE || op == A_GT || op == A_LE || op == A_LT || op == A_EQ || op == A_NE || op == A_LSHIFT || op == A_RSHIFT) {
                type = P_LONG; // Comparison operations return an integer type
            }
            else if (is_pointer(left->getPrimitiveType()) || is_pointer(right->getPrimitiveType())) {
                // 表达式中不允许出现两个指针类型的操作数
                type = is_pointer(left->getPrimitiveType()) ? left->getPrimitiveType() : right->getPrimitiveType();
            } else {
                type = cal_type; // Otherwise, use the calculated type
            }
        }
        PrimitiveType getCalType() const {
            return cal_type; // Return the calculated type
        }

        void setRight(std::shared_ptr<ExprNode> right) {
            this->right = std::move(right); // Set the right operand of the binary expression
        }
        void setLeft(std::shared_ptr<ExprNode> left) {
            this->left = std::move(left); // Set the left operand of the binary expression
        }
    private:
        ExprType op;
        PrimitiveType cal_type;
        std::shared_ptr<ExprNode> left;
        std::shared_ptr<ExprNode> right;
};


class UnaryExpNode : public ExprNode {
    public:
        UnaryExpNode(UnaryOp op, std::shared_ptr<ExprNode> expr, PrimitiveType type): op(op), expr(std::move(expr)) {
            this->type = type;
        }
        UnaryExpNode(TokenType tok, std::shared_ptr<ExprNode> expr) {
            if (tok == T_PLUS) {
                op = U_PLUS;
            } else if (tok == T_MINUS) {
                op = U_MINUS;
            } else if (tok == T_NOT) {
                op = U_NOT;
            } else if (tok == T_INVERT) {
                op = U_INVERT;
            } else {
                throw std::runtime_error("UnaryExpNode: Unknown token type for unary operation");
            }
            this->expr = std::move(expr);
        }
        UnaryOp getOp() const { return op; }
        std::shared_ptr<ExprNode> getExpr() const { return expr; }
        std::string convertTypeToString() const {
            switch (op) {
                case U_PLUS:
                    return "Plus";
                case U_MINUS:
                    return "Minus";
                case U_NOT:
                    return "Not";
                case U_ADDR:
                    return "Address of";
                case U_DEREF:
                    return "Dereference";
                case U_TRANSFORM:
                    if (type == P_INT) {
                        return "Transform, Target Type: int";
                    } else if (type == P_FLOAT) {
                        return "Transform, Target Type: float";
                    } else if (type == P_CHAR) {
                        return "Transform, Target Type: char";
                    } else if (type == P_LONG) {
                        return "Transform, Target Type: long";
                    } else if (type == P_VOID) {
                        return "Transform, Target Type: void";
                    }
                    return "Transform, Target Type: " + std::to_string(type);
                case U_SCALE:
                    return "Scale, Offset: " + std::to_string(getOffset());
                case U_PREINC:
                    return "Pre-increment";
                case U_PREDEC:
                    return "Pre-decrement";
                case U_POSTINC:
                    return "Post-increment";
                case U_POSTDEC:
                    return "Post-decrement";
                default:
                    return "Unknown Unary Type";
            }
        }
        void walk(std::string prefix) override {
            // Implement the walk method to traverse the unary expression node
            // This could involve visiting the operand, etc.
            std::cout << prettyPrint(prefix) << "Unary Expr, Op " << convertTypeToString() << " :" << std::endl;
            expr->walk(prefix + "\t");
            // if (op == U_MINUS) {
            //     value = -expr->getValue(); // Negate the value for unary minus
            // } else {
            //     value = expr->getValue(); // For unary plus, just return the value
            // }
        }
        void updateType() {
            if (op != U_TRANSFORM && op != U_ADDR && op != U_DEREF) type = expr->getPrimitiveType();
            if (op == U_SCALE || op == U_NOT) type = P_LONG; // Scale operation always results in a long type
        }
    private:
        UnaryOp op;
        std::shared_ptr<ExprNode> expr;
};

class LValueNode : public ExprNode {
    public:
        LValueNode(std::shared_ptr<Symbol> identifier, std::shared_ptr<ExprNode> index = nullptr): identifier(identifier) {
            type = identifier->type;
            if (identifier->is_array) {
                is_array = true; // Set the flag if the identifier is an array
            }
            this->index = std::move(index); // Set the index if provided
        }
        std::string convertTypeToString() const {
            switch (type) {
                case P_INT: return "int";
                case P_FLOAT: return "float";
                case P_CHAR: return "char";
                case P_LONG: return "long";
                case P_VOID: return "void";
                case P_INTPTR: return "int*";
                case P_FLOATPTR: return "float*";
                case P_CHARPTR: return "char*";
                case P_LONGPTR: return "long*";
                case P_VOIDPTR: return "void*";
                case P_INTARR: return "int[]" ;
                case P_FLOATARR: return "float[]";
                case P_CHARARR: return "char[]" ;
                case P_LONGARR: return "long[]";
                default: return "unknown";
            }
        }
        Symbol getIdentifier() const { 
            Symbol sym = *identifier; // Return a copy of the identifier symbol
            return sym; 
        }
        void walk(std::string prefix) override {
            // Implement the walk method to print the identifier
            std::cout << prettyPrint(prefix) << "LValue Identifier: " << identifier->name << ", Type: " << convertTypeToString() << std::endl;
            if (is_array && index) {
                index->walk(prefix + "\t");
            }
        }
        bool isArray() const {
            return is_array; // Return whether the identifier is an array
        }

        bool isParam() const {
            return identifier->is_param; // Return whether the identifier is a parameter
        }
        std::shared_ptr<ExprNode> getIndex() const {
            return index; // Return the index expression if it exists
        }
        void setIndex(std::shared_ptr<ExprNode> index) {
            this->index = std::move(index); // Set the index expression for the identifier
        }

        void setIndexLen(int len) {
            index_len = len;
        }

        int getIndexLen() const {
            return index_len; // Return the length of the index if it is an array
        }
    private:
        std::shared_ptr<Symbol> identifier;
        bool is_array = false; // Flag to indicate if the identifier is an array
        std::shared_ptr<ExprNode> index;
        int index_len = 0; // Length of the index if it is an array, used for array initialization
};


class FunctionCallNode : public ExprNode {
    public:
        FunctionCallNode(std::string identifier, std::vector<std::shared_ptr<ExprNode>> args, PrimitiveType return_type)
            : identifier(std::move(identifier)), args(std::move(args)) {
            type = return_type; // Set the type of the function call to void
        }

        void walk(std::string prefix) override {
            std::cout << prettyPrint(prefix) << "Function Call: " << identifier << std::endl;
            for (const auto& arg : args) {
                arg->walk(prefix + "\t"); // Walk each argument of the function call
            }
        }

        std::string getIdentifier() const {
            return identifier; // Return the function identifier
        }

        std::vector<std::shared_ptr<ExprNode>> getArguments() const {
            return args; // Return the list of arguments for the function call
        }

    private:
        std::string identifier; // Identifier for the function being called
        std::vector<std::shared_ptr<ExprNode>> args; // Arguments for the function call
};

class AssignmentNode : public ExprNode {
    public:
        AssignmentNode(std::shared_ptr<ExprNode> lvalue, std::shared_ptr<ExprNode> expr) 
            : lvalue(std::move(lvalue)), expression(std::move(expr)) {
            type = this->lvalue->getCalculateType(); // Set the type of the assignment to the identifier's type
        }
        void walk(std::string prefix) override {
            std::cout << prettyPrint(prefix) << "Assignment Statement: "<< std::endl;
            lvalue->walk(prefix + "\t"); // Walk the expression being assigned
            expression->walk(prefix + "\t"); // Walk the expression node
        }

        std::shared_ptr<ExprNode> getLvalue() const {
            return lvalue;
        }

        void setType(PrimitiveType type) {
            this->type = type; // Set the type of the assignment node
        }

        std::shared_ptr<ExprNode> getExpr() const {
            return expression; // Return the expression being assigned
        }
        void setExpr(std::shared_ptr<ExprNode> expr) {
            expression = std::move(expr); // Set the expression being assigned
        }   

    private:

        std::shared_ptr<ExprNode> lvalue;
        std::shared_ptr<ExprNode> expression; // Expression to assign to the variable
};

class ArrayInitializer : public ExprNode {
    public:
        ArrayInitializer(std::shared_ptr<Symbol> sym, std::vector<int> &dims, int depth) {
            if ((size_t)depth != dims.size() - 1) {
                min_size = dims.back();
                max_size = 1;
                for (size_t i = depth; i < dims.size(); ++i) {
                    max_size *= dims[i]; // Calculate the maximum size of the array
                }
                next_size = max_size / dims[depth];
            } else {
                min_size = 0;
                max_size = dims[depth];
                next_size = -1;
            }
            current_size = 0;
            symbol = std::move(sym); // Store the symbol for the array being initialized
        }

        void addInitializer(std::shared_ptr<ExprNode> value) {
            if (auto x = std::dynamic_pointer_cast<ArrayInitializer>(value)) {
                if (next_size == -1) {
                    throw std::runtime_error("ArrayInitializer::addInitializer: too many initializers");
                }
                current_size += next_size;
                values.push_back(x);
            } else if (auto x = std::dynamic_pointer_cast<ValueNode>(value)) {
                values.push_back(x);
                current_size++;
            } 
            if (current_size > max_size) {
                throw std::runtime_error("ArrayInitializer::addInitializer: too many initializers");
            }
        }

        bool canAcceptNestedInitializer() const {
            return current_size % min_size == 0;
        }

        void walk(std::string prefix) override {
            std::cout << prettyPrint(prefix) << "Array Initializer: " << std::endl;
            for (const auto& value : values) {
                value->walk(prefix + "\t"); // Walk each value in the array initializer
            }
        }

        std::vector<std::shared_ptr<ExprNode>> getElements() const {
            return values; // Return the values in the array initializer
        }

        int getLeftSize() const {
            return max_size - current_size; // Return the number of elements that can still be added
        }

        void setBaseOffset() {
            base_offset = symbol->pos_in_stack; // Set the base offset for the array initializer
        }

        void setBaseOffset(int offset) {
            base_offset = offset; // Set the base offset for the array initializer
        }

        void getValuePos() {
            int elem_size = symbol_table.typeToSize(type);
            int j = 0;
            for (size_t i = 0; i < values.size(); ++i) {
                if (auto x = std::dynamic_pointer_cast<ValueNode>(values[i])) {
                    pos_on_stack.push_back(base_offset + j * elem_size); // Calculate the position of the value on the stack
                    j++;
                } else if (auto x = std::dynamic_pointer_cast<ArrayInitializer>(values[i])) {
                    x->setBaseOffset(base_offset + j * elem_size); // Set the base offset for nested initializers
                    // x->getValuePos(); // Recursively get the positions for nested initializers
                    j += next_size;
                }
            }
        }

        std::vector<int> getPosOnStack() const {
            return pos_on_stack; // Return the positions of the values on the stack
        }

    private:
        int min_size;
        int current_size; // 现在初始化的元素个数
        int next_size; //遇到一个{}，之后current_size需要增加的size
        int max_size;
        int base_offset;
        std::shared_ptr<Symbol> symbol; // Symbol for the array being initialized
        // 只允许常量声明
        std::vector<std::shared_ptr<ExprNode>> values; // Values to initialize the array with
        std::vector<int> pos_on_stack; // Positions of the values on the stack

};