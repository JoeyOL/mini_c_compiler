#include "common/defs.h"
#include <memory>  
#include <iostream>
#include <map>
#pragma once

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

        void updateCalType() {

            // // Update the type based on the left and right expressions
            // if (left->getType() == right->getType() ) {
            //     type = left->getType(); // If both types are the same, use that type
            //     return;
            // }
            if (left->getType() == P_FLOAT || right->getType() == P_FLOAT) {
                cal_type = P_FLOAT;
            } else if (left->getType() == P_LONG || right->getType() == P_LONG) {
                cal_type = P_LONG; // If either is long, the result is long
            } else if (left->getType() == P_INT || right->getType() == P_INT) {
                cal_type = P_INT;
                if (op == A_GE || op == A_GT || op == A_LE || op == A_LT || op == A_EQ || op == A_NE) {
                    cal_type = P_LONG; // Comparison operations return an integer type
                    return;
                }
            } else if (left->getType() == P_CHAR || right->getType() == P_CHAR) {
                cal_type = P_CHAR; // If either is char, the result is char
                if (op == A_GE || op == A_GT || op == A_LE || op == A_LT || op == A_EQ || op == A_NE) {
                    cal_type = P_LONG; // Comparison operations return an integer type
                    return;
                }
            } 
        }

        void updateTypeAfterCal() {
            if (op == A_GE || op == A_GT || op == A_LE || op == A_LT || op == A_EQ || op == A_NE) {
                type = P_LONG; // Comparison operations return an integer type
            }
            else {
                type = cal_type;
            }
        }
        PrimitiveType getCalType() const {
            return cal_type; // Return the calculated type
        }
    private:
        ExprType op;
        PrimitiveType cal_type;
        std::shared_ptr<ExprNode> left;
        std::shared_ptr<ExprNode> right;
};


class UnaryExpNode : public ExprNode {
    public:
        UnaryExpNode(UnaryOp op, std::shared_ptr<ExprNode> expr): op(op), expr(std::move(expr)) {
            type = P_NONE;
        }
        UnaryExpNode(TokenType tok, std::shared_ptr<ExprNode> expr) {
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
        std::shared_ptr<ExprNode> getExpr() const { return expr; }
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
        void updateType() {
            type = expr->getType();
        }
    private:
        UnaryOp op;
        std::shared_ptr<ExprNode> expr;
};

class LValueNode : public ExprNode {
    public:
        LValueNode(Symbol identifier) : identifier(std::move(identifier)) {
            type = identifier.type;
        }
        Symbol getIdentifier() const { return identifier; }
        void walk(std::string prefix) override {
            // Implement the walk method to print the identifier
            std::cout << prettyPrint(prefix) << "LValue Identifier: " << identifier.name << std::endl;
        }
    private:
        Symbol identifier;
};
