#include "common/defs.h"
#include <memory>
#include <iostream>  
#include "parser/ExpNode.h"
#include <vector>
#pragma once

class StatementsNode : public StatementNode {
    public:
        StatementsNode() = default;
        ~StatementsNode() = default;
        void walk() override {
            std::cout << "Walking through statements node." << std::endl;
            for (const auto& stmt : statements) {
                stmt->walk(); // Walk each statement
            }
        }
        void addStatement(std::shared_ptr<StatementNode> stmt) {
            statements.push_back(std::move(stmt)); // Add a new statement
        }
        std::vector<std::shared_ptr<StatementNode>> getStatements() const {
            return statements; // Return the list of statements
        }
    private:
        std::vector<std::shared_ptr<StatementNode>> statements; // List of statements
};

class PrintStatementNode : public StatementNode {
    public:
        PrintStatementNode(std::shared_ptr<ExprNode> expr) : expression(std::move(expr)) {
            type = S_PRINT; // Set the statement type to print
        }
        void walk() override {
            std::cout << "Print Statement: " << std::endl;
            expression->walk(); // Walk the expression node
        }
        Value getValue() override {
            return expression->getValue(); // Return the value of the expression
        }
        std::shared_ptr<ExprNode> getExpression() const {
            return expression; // Return the expression to print
        }
    private:
        std::shared_ptr<ExprNode> expression; // Expression to print
};

