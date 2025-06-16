#include "common/defs.h"
#include <memory>
#include <iostream>  
#include "parser/ExprNode.h"

#pragma once

class BlockNode : public StatementNode {
    public:
        BlockNode() {
            type = S_BLOCK; // Set the statement type to block
        };
        ~BlockNode() = default;
        void walk(std::string prefix) override {
            std::cout << prettyPrint(prefix) << "Block node :" << std::endl;
            for (const auto& stmt : statements) {
                stmt->walk(prefix + "\t"); // Walk each statement
            }
        }
        void addStatement(std::shared_ptr<StatementNode> stmt) {
            statements.push_back(std::move(stmt)); // Add a new statement
        }
        std::vector<std::shared_ptr<StatementNode>> getStatements() const {
            return statements; // Return the list of statements
        }
        bool is_labeled = false; // Flag to indicate if the block has a label

    private:
        std::vector<std::shared_ptr<StatementNode>> statements; // List of statements
};

class PrintStatementNode : public StatementNode {
    public:
        PrintStatementNode(std::shared_ptr<ExprNode> expr) : expression(std::move(expr)) {
            type = S_PRINT; // Set the statement type to print
        }
        void walk(std::string prefix) override {
            std::cout << prettyPrint(prefix) << "Print Statement: " << std::endl;
            expression->walk(prefix + "\t"); // Walk the expression node
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

class VariableDeclareNode : public StatementNode {
    public:
        VariableDeclareNode(PrimitiveType var_type) : var_type(var_type) {
            this->type = S_VARDEF; // Set the statement type to variable declaration
        }

        std::string convertTypeToString(PrimitiveType type) const {
            switch (type) {
                case P_INT: return "int";
                case P_FLOAT: return "float";
                case P_STRING: return "string";
                case P_BOOL: return "bool";
                default: return "unknown";
            }
        }

        void walk(std::string prefix) override {
            std::cout << prettyPrint(prefix) << "Declare Statement, Type " << convertTypeToString(var_type) << ", Name: ";
            for (auto &identifier : identifiers) {
                std::cout << identifier << ", ";
            }
            std::cout << std::endl;
        }

        void addIdentifier(std::string identifier) {
            identifiers.push_back(identifier); // Add a new identifier
        }

        void addIdentifier(std::string identifier, std::shared_ptr<ExprNode> initializer) {
            identifiers.push_back(identifier); // Add a new identifier
            initializers[identifier] = std::move(initializer); // Add the initializer for the identifier
        }

        std::vector<std::string> getIdentifiers() const {
            return identifiers; // Return the list of identifiers
        }
        std::shared_ptr<ExprNode> getInitializer(std::string identifier) const {
            if (initializers.find(identifier) != initializers.end()) {
                return initializers.at(identifier); // Return the initializer for the identifier
            }
            return nullptr; // Return nullptr if no initializer exists for the identifier
        }
    private:
        PrimitiveType var_type;
        std::vector<std::string> identifiers;
        std::map<std::string, std::shared_ptr<ExprNode>> initializers;
};

class AssignmentNode : public StatementNode {
    public:
        AssignmentNode(Symbol identifier, std::shared_ptr<ExprNode> expr) 
            : identifier(std::move(identifier)), expression(std::move(expr)) {
            type = S_ASSIGN; // Set the statement type to assignment
        }
        void walk(std::string prefix) override {
            std::cout << prettyPrint(prefix) << "Assignment Statement: " << identifier.name << " = " << std::endl;
            expression->walk(prefix + "\t"); // Walk the expression node
        }
        Value getValue() override {
            return expression->getValue(); // Return the value of the expression
        }
        Symbol getIdentifier() const {
            return identifier; // Return the identifier being assigned to
        }
        std::shared_ptr<ExprNode> getExpr() const {
            return expression; // Return the expression being assigned
        }
    private:
        Symbol identifier; // Identifier for the variable being assigned
        std::shared_ptr<ExprNode> expression; // Expression to assign to the variable
};


class IfStatementNode : public StatementNode {
    public:
        IfStatementNode(std::shared_ptr<ExprNode> condition, 
                        std::shared_ptr<StatementNode> then_stmt, 
                        std::shared_ptr<StatementNode> else_stmt = nullptr)
            : condition(std::move(condition)), then_stmt(std::move(then_stmt)), else_stmt(std::move(else_stmt)) {
            type = S_IF; // Set the statement type to if
        }

        void walk(std::string prefix) override {
            std::cout << prettyPrint(prefix) << "If Statement: " << std::endl;
            condition->walk(prefix + "\t"); // Walk the condition expression
            then_stmt->walk(prefix + "\t"); // Walk the then statement
            if (else_stmt) {
                std::cout << prettyPrint(prefix + "\t") << "Else Statement: " << std::endl;
                else_stmt->walk(prefix + "\t\t"); // Walk the else statement if it exists
            }
        }

        std::shared_ptr<ExprNode> getCondition() const {
            return condition; // Return the condition expression
        }

        std::shared_ptr<StatementNode> getThenStatement() const {
            return then_stmt; // Return the then statement
        }

        std::shared_ptr<StatementNode> getElseStatement() const {
            return else_stmt; // Return the else statement if it exists
        }
    private:
        std::shared_ptr<ExprNode> condition;
        std::shared_ptr<StatementNode> then_stmt;
        std::shared_ptr<StatementNode> else_stmt;
};

class WhileStatementNode : public StatementNode {
    public:
        WhileStatementNode(std::shared_ptr<ExprNode> condition, std::shared_ptr<StatementNode> body)
            : condition(std::move(condition)), body(std::move(body)) {
            type = S_WHILE; // Set the statement type to while
        }

        void walk(std::string prefix) override {
            std::cout << prettyPrint(prefix) << "While Statement: " << std::endl;
            condition->walk(prefix + "\t"); // Walk the condition expression
            body->walk(prefix + "\t"); // Walk the body of the while loop
        }

        std::shared_ptr<ExprNode> getCondition() const {
            return condition; // Return the condition expression
        }

        std::shared_ptr<StatementNode> getBody() const {
            return body; // Return the body of the while loop
        }
    private:
        std::shared_ptr<ExprNode> condition; // Condition for the while loop
        std::shared_ptr<StatementNode> body; // Body of the while loop
};