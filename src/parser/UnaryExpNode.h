#include "common/defs.h"
#include <memory>
#include <iostream>  
#pragma once

class UnaryExpNode : public ExprNode {
    public:
        UnaryExpNode(UnaryOp op, std::shared_ptr<ASTNode> expr): op(op), expr(std::move(expr)) {
            if (this->expr->getType() == T_FLOATLIT) {
                value.setFloatValue(0.0); // Initialize to float if the expression is float
            } else {
                value.setIntValue(0); // Initialize to int otherwise
            }
        }
        UnaryOp getOp() const { return op; }
        std::shared_ptr<ASTNode> getExpr() const { return expr; }
        std::string convertTypeToString() const {
            switch (op) {
                case U_PLUS:
                    return "Unary Plus Expression";
                case U_MINUS:
                    return "Unary Minus Expression";
                default:
                    return "Unknown Unary Type";
            }
        }
        void walk() override {
            // Implement the walk method to traverse the unary expression node
            // This could involve visiting the operand, etc.
            expr->walk();
            if (op == U_MINUS) {
                value = -expr->getValue(); // Negate the value for unary minus
            } else {
                value = expr->getValue(); // For unary plus, just return the value
            }
            std::cout << "Unary Expression: " << convertTypeToString() << "Result: " << value.toString() << std::endl;
        }
    private:
        UnaryOp op;
        std::shared_ptr<ASTNode> expr;
};