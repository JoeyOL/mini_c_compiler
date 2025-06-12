#include "common/defs.h"
#include <memory>
#include <iostream>  
#pragma once

class UnaryExpNode : public ASTNode {
    public:
        UnaryExpNode(UnaryType type, int value): type(type) {
            this->value = value; // Assuming ASTNode has a value field
        }
        UnaryType getType() const { return type; }
        int getValue() const { return value; }
        std::string convertTypeToString() const {
            switch (type) {
                case U_INT:
                    return "Unary Integer";
                default:
                    return "Unknown Unary Type";
            }
        }
        void walk() override {
            // Implement the walk method to traverse the unary expression node
            // This could involve visiting the operand, etc.
            std::cout << "Unary Expression: " << convertTypeToString() << " with value: " << value << std::endl;
        }
    private:
        UnaryType type;
};