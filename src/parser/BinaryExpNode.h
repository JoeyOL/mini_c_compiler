#include "common/defs.h"
#include <memory>  
#include <iostream>
#include <map>
#pragma once

class BinaryExpNode : public ExprNode {
    public:
        BinaryExpNode(ExprType type, std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right)
            : type(type), left(std::move(left)), right(std::move(right)) {
                if (this->left->getType() == T_FLOATLIT || 
                    this->right->getType() == T_FLOATLIT) {
                    value.setFloatValue(0.0); // Initialize to float if any operand is float
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
            };
            auto it = typeToString.find(type);
            if (it != typeToString.end()) {
                return it->second;
            } else {
                return "Unknown Type";
            }
        }
        void walk() override {
            // Implement the walk method to traverse the binary expression node
            // This could involve visiting the left and right nodes, etc.
            left->walk();
            right->walk();
            std::cout << "Binary Expression: " << convertTypeToString();
            std::cout << " (Left: " << (left->getValue().toString()) 
                      << ", Right: " << (right->getValue().toString()) << ")";
            switch (type) {
                case A_ADD:
                    value = left->getValue() + right->getValue();
                    break;
                case A_SUBTRACT:
                    value = left->getValue() - right->getValue();
                    break;
                case A_MULTIPLY:
                    value = left->getValue() * right->getValue();
                    break;
                case A_DIVIDE:
                    if (right->getValue() == Value{T_INTLIT, .ivalue = 0} 
                        || right->getValue() == Value{T_FLOATLIT, .fvalue = 0.0}) {
                        throw std::runtime_error("Division by zero error");
                    }
                    value = left->getValue() / right->getValue();
                    break;
                default:
                    throw std::runtime_error("Unknown binary expression type");
            }
            std::cout << " Result: " << value.toString() << std::endl;
        }

    private:
        ExprType type;
        std::shared_ptr<ASTNode> left;
        std::shared_ptr<ASTNode> right;
};