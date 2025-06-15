#include "common/defs.h"
#include <memory>
#include <iostream>  
#include "parser/ExprNode.h"

#pragma once

class StatementsNode : public StatementNode {
    public:
        StatementsNode() = default;
        ~StatementsNode() = default;
        void walk(std::string prefix) override {
            std::cout << prettyPrint(prefix) << "Statements node :" << std::endl;
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
