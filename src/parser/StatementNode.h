#include "common/defs.h"
#include <memory>
#include <iostream>  
#include "parser/ExprNode.h"

#pragma once

class BlockNode : public StatementNode {
    public:
        BlockNode() {
            stmt_type= S_BLOCK; // Set the statement type to block
            type = P_NONE; // Set the type of the block to void
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
            stmt_type= S_PRINT; // Set the statement type to print
        }
        void walk(std::string prefix) override {
            std::cout << prettyPrint(prefix) << "Print Statement: " << std::endl;
            expression->walk(prefix + "\t"); // Walk the expression node
        }

        std::shared_ptr<ExprNode> getExpression() const {
            return expression; // Return the expression to print
        }
        void setType(PrimitiveType type) {
            this->type = type; // Set the type of the expression node
        }
    private:
        std::shared_ptr<ExprNode> expression; // Expression to print
};

class VariableDeclareNode : public StatementNode {
    public:
        VariableDeclareNode(PrimitiveType var_type) : var_type(var_type) {
            this->stmt_type= S_VARDEF; // Set the statement type to variable declaration
            type = P_NONE; // Set the type of the variable
        }

        VariableDeclareNode(TokenType tok_type) {
            this->stmt_type= S_VARDEF; // Set the statement type to variable declaration
            if (tok_type == T_INT) {
                var_type = P_INT; // Set the variable type to int
            } else if (tok_type == T_CHAR) {
                var_type = P_CHAR; // Set the variable type to char
            } else if (tok_type == T_FLOAT) {
                var_type = P_FLOAT; // Set the variable type to float
            } else if (tok_type == T_LONG) {
                var_type = P_LONG;
            } else {
                throw std::runtime_error("VariableDeclareNode: Unknown token type for variable declaration");
            }
        }

        std::string convertTypeToString(PrimitiveType type) const {
            switch (type) {
                case P_INT: return "int";
                case P_FLOAT: return "float";
                case P_CHAR: return "char";
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

        PrimitiveType getVariableType() const {
            return var_type; // Return the type of the variable
        }

    private:
        PrimitiveType var_type;
        std::vector<std::string> identifiers;
        std::map<std::string, std::shared_ptr<ExprNode>> initializers;
};

class AssignmentNode : public StatementNode, public ExprNode {
    public:
        AssignmentNode(Symbol identifier, std::shared_ptr<ExprNode> expr) 
            : identifier(std::move(identifier)), expression(std::move(expr)) {
            stmt_type= S_ASSIGN; // Set the statement type to assignment
            ExprNode::type = identifier.type; // Set the type of the assignment to the identifier's type
        }
        void walk(std::string prefix) override {
            std::cout << prettyPrint(prefix) << "Assignment Statement: " << identifier.name << " = " << std::endl;
            expression->walk(prefix + "\t"); // Walk the expression node
        }

        Symbol getIdentifier() const {
            return identifier; // Return the identifier being assigned to
        }
        std::shared_ptr<ExprNode> getExpr() const {
            return expression; // Return the expression being assigned
        }
        PrimitiveType getType() const override {
            return ExprNode::type; // Return the type of the expression node
        }
        bool & isNeedTransform() override {
            return ExprNode::need_transform; // Return whether the assignment needs transformation
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
            stmt_type= S_IF; // Set the statement type to if
            type = P_NONE; // Set the type of the if statement to void
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
            stmt_type= S_WHILE; // Set the statement type to while
            type = P_NONE; // Set the type of the while statement to void
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


class ForStatementNode : public StatementNode {
    public:
        ForStatementNode(std::shared_ptr<StatementNode> preop_stmt, std::shared_ptr<ExprNode> condition, std::shared_ptr<StatementNode> body, std::shared_ptr<StatementNode> postop_stmt)
            : condition(std::move(condition)), body(std::move(body)), preop_stmt(std::move(preop_stmt)), postop_stmt(std::move(postop_stmt)) {
            stmt_type = S_FOR; // Set the statement type to while
            type = P_NONE; // Set the type of the for statement to void
        }

        void walk(std::string prefix) override {
            std::cout << prettyPrint(prefix) << "For Statement: " << std::endl;
            if (preop_stmt) {
                preop_stmt->walk(prefix + "\t"); // Walk the pre-work statement of the for loop
            }
            condition->walk(prefix + "\t"); // Walk the condition expression
            body->walk(prefix + "\t"); // Walk the body of the while loop
        }

        std::shared_ptr<ExprNode> getCondition() const {
            return condition; // Return the condition expression
        }

        std::shared_ptr<StatementNode> getBody() const {
            return body; // Return the body of the while loop
        }

        std::shared_ptr<StatementNode> getPreopStatement() const {
            return preop_stmt; // Return the pre-operation statement
        }
        std::shared_ptr<StatementNode> getPostopStatement() const {
            return postop_stmt; // Return the post-operation statement
        }
    private:
        std::shared_ptr<StatementNode> preop_stmt;
        std::shared_ptr<ExprNode> condition; // Condition for the while loop
        std::shared_ptr<StatementNode> body; // Body of the while loop
        std::shared_ptr<StatementNode> postop_stmt;

};

// TODO: 暂时不支持带参数的函数
class FunctionDeclareNode : public  StatementNode {
    public:
        FunctionDeclareNode(std::string identifier, PrimitiveType return_type, std::shared_ptr<BlockNode> body)
            : identifier(std::move(identifier)), return_type(return_type), body(std::move(body)) {
            stmt_type = S_FUNCTDEF; // Set the statement type to variable declaration
            type = P_NONE;
        }

        std::string convertTypeToString(PrimitiveType type) const {
            switch (type) {
                case P_INT: return "int";
                case P_FLOAT: return "float";
                case P_VOID: return "void";
                default: return "unknown";
            }
        }

        void walk(std::string prefix) override {
            std::cout << prettyPrint(prefix) << "Function Declaration: " << identifier << ", Return Type: " 
                      << convertTypeToString(return_type) << std::endl;
            body->walk(prefix + "\t"); // Walk the function body
        }

        std::string getIdentifier() const {
            return identifier; // Return the function identifier
        }

        PrimitiveType getReturnType() const {
            return return_type; // Return the function return type
        }

        std::shared_ptr<BlockNode> getBody() const {
            return body; // Return the function body
        }

    private:
        std::string identifier;
        PrimitiveType return_type;
        std::shared_ptr<BlockNode> body;
};