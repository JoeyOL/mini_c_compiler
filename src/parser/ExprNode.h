#include "common/defs.h"
#include <memory>  
#include <iostream>
#include <map>
#pragma once

class BinaryExpNode : public ExprNode {
    public:
        BinaryExpNode(ExprType type, std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right)
            : type(type), left(std::move(left)), right(std::move(right)) {
                if (this->left->getType() == P_FLOAT || 
                    this->right->getType() == P_FLOAT) {
                    value.setFloatValue(0.0); // Initialize to float  if any operand is float
                } else {
                    value.setIntValue(0); // Initialize to int otherwise
                }
            }

        ExprType getType() const { return type; }
        std::shared_ptr<ASTNode> getLeft() const { return left; }
        std::shared_ptr<ASTNode> getRight() const { return right; }
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
                {A_GE, "Greater Than or Equal"}
            };
            auto it = typeToString.find(type);
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
            // switch (type) {
            //     case A_ADD:
            //         value = left->getValue() + right->getValue();
            //         break;
            //     case A_SUBTRACT:
            //         value = left->getValue() - right->getValue();
            //         break;
            //     case A_MULTIPLY:
            //         value = left->getValue() * right->getValue();
            //         break;
            //     case A_DIVIDE:
            //         if (right->getValue() == Value{P_INT, .ivalue = 0} 
            //             || right->getValue() == Value{P_FLOAT, .fvalue = 0.0}) {
            //             throw std::runtime_error("Division by zero error");
            //         }
            //         value = left->getValue() / right->getValue();
            //         break;
            //     default:
            //         throw std::runtime_error("Unknown binary expression type");
            // }
            // std::cout << " Result: " << value.toString() << std::endl;
        }

    private:
        ExprType type;
        std::shared_ptr<ASTNode> left;
        std::shared_ptr<ASTNode> right;
};


class UnaryExpNode : public ExprNode {
    public:
        UnaryExpNode(UnaryOp op, std::shared_ptr<ASTNode> expr): op(op), expr(std::move(expr)) {}
        UnaryExpNode(TokenType tok, std::shared_ptr<ASTNode> expr) {
            if (tok == T_PLUS) {
                op = U_PLUS;
            } else if (tok == T_MINUS) {
                op = U_MINUS;
            } else if (tok == T_NOT) {
                op = U_NOT;
            } else {
                throw std::runtime_error("UnaryExpNode: Unknown token type for unary operation");
            }
            this->expr = std::move(expr);
        }
        UnaryOp getOp() const { return op; }
        std::shared_ptr<ASTNode> getExpr() const { return expr; }
        std::string convertTypeToString() const {
            switch (op) {
                case U_PLUS:
                    return "Plus";
                case U_MINUS:
                    return "Minus";
                case U_NOT:
                    return "Not";
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
    private:
        UnaryOp op;
        std::shared_ptr<ASTNode> expr;
};

class LValueNode : public ExprNode {
    public:
        LValueNode(Symbol identifier) : identifier(std::move(identifier)) {}
        Symbol getIdentifier() const { return identifier; }
        void walk(std::string prefix) override {
            // Implement the walk method to print the identifier
            std::cout << prettyPrint(prefix) << "LValue Identifier: " << identifier.name << std::endl;
        }
    private:
        Symbol identifier;
};
