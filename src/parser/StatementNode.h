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

        void setExpression(std::shared_ptr<ExprNode> expr) {
            expression = std::move(expr); // Set the expression to print
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
        void setVariableType(PrimitiveType type) {
            this->var_type = type; // Set the variable type
        }

        std::string convertDimensionsToString() const {
            std::string ret = "[";
            for (int dim : dimensions) {
                ret += std::to_string(dim) + "][";
            }
            ret.pop_back(); // Remove the last '['
            ret += "]";
            return ret; // Return the string representation of the dimensions
        }

        std::string convertTypeToString(PrimitiveType type) const {
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
                case P_INTARR: return "int" + convertDimensionsToString();
                case P_FLOATARR: return "float" + convertDimensionsToString();
                case P_CHARARR: return "char" + convertDimensionsToString();
                case P_LONGARR: return "long" + convertDimensionsToString();
                default: return "unknown";
            }
        }

        void walk(std::string prefix) override {
            std::cout << prettyPrint(prefix) << "Declare Statement, Type " << convertTypeToString(var_type) << ", Name: ";
            for (auto &identifier : identifiers) {
                std::cout << identifier << ", ";
            }
            std::cout << std::endl;
            for (const auto& [identifier, initializer] : initializers) {
                std::cout << prettyPrint(prefix + "\t") << "Initializer for " << identifier << ": " << std::endl;
                if (initializer) {
                    initializer->walk(prefix + "\t\t"); // Walk the initializer expression
                } else {
                    std::cout << "No initializer" << std::endl; // No initializer provided
                }
            }
        }

        void addIdentifier(std::string identifier) {
            identifiers.push_back(identifier); // Add a new identifier
        }

        void addIdentifier(std::string identifier, std::shared_ptr<ExprNode> initializer) {
            identifiers.push_back(identifier); // Add a new identifier
            initializers[identifier] = std::move(initializer); // Add the initializer for the identifier
        }

        void setInitializer(std::string identifier, std::shared_ptr<ExprNode> initializer) {
            initializers[identifier] = std::move(initializer); // Set the initializer for the identifier
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

        void setArray(bool is_array) {
            this->is_array = is_array; // Set whether the variable is an array
            if (is_array) {
                if (var_type == P_INT) {
                    var_type = P_INTARR; // Set the type to int array
                } else if (var_type == P_FLOAT) {
                    var_type = P_FLOATARR; // Set the type to float array
                } else if (var_type == P_CHAR) {
                    var_type = P_CHARARR; // Set the type to char array
                } else if (var_type == P_LONG) {
                    var_type = P_LONGARR; // Set the type to long array
                }
            }
        }

        bool isArray() const {
            return is_array; // Return whether the variable is an array
        }

        void addDimension(int dimension) {
            dimensions.push_back(dimension); // Add a new dimension for array variables
        }
        std::vector<int> getDimensions() const {
            return dimensions; // Return the dimensions for array variables
        }

    private:
        std::vector<int> dimensions = {}; // Dimensions for array variables
        bool is_array = false;
        PrimitiveType var_type;
        int star_count; // 指针类型
        std::vector<std::string> identifiers;
        std::map<std::string, std::shared_ptr<ExprNode>> initializers;
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

        FunctionDeclareNode(std::string identifier, TokenType return_type, std::shared_ptr<BlockNode> body) {
            this->stmt_type = S_FUNCTDEF; // Set the statement type to variable declaration
            this->body = std::move(body);
            this->identifier = std::move(identifier);
            if (return_type == T_INT) {
                this->return_type = P_INT; // Set the return type to int
            } else if (return_type == T_FLOAT) {
                this->return_type = P_FLOAT; // Set the return type to float
            } else if (return_type == T_CHAR) {
                this->return_type = P_CHAR; // Set the return type to char
            } else if (return_type == T_LONG) {
                this->return_type = P_LONG;
            } else if (return_type == T_VOID) {
                this->return_type = P_VOID; // Set the return type to void
            } else {
                throw std::runtime_error("FunctionDeclareNode: Unknown token type for function declaration");
            }
            this->type = P_NONE; // Set the type of the function declaration to void    
        }

        std::string convertTypeToString(PrimitiveType type) const {
            switch (type) {
                case P_INT: return "int";
                case P_FLOAT: return "float";
                case P_CHAR: return "char";
                case P_LONG: return "long";
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




// 不考虑出现由于控制流导致不return的情况
class ReturnStatementNode : public StatementNode {
    public:
        ReturnStatementNode(std::shared_ptr<ExprNode> expr) : expression(std::move(expr)) {
            stmt_type = S_RETURN; // Set the statement type to return
            type = P_NONE; // Set the type of the return statement to void
        }

        void walk(std::string prefix) override {
            std::cout << prettyPrint(prefix) << "Return Statement: " << std::endl;
            if (expression) {
                expression->walk(prefix + "\t"); // Walk the expression being returned
            } else {
                std::cout << prettyPrint(prefix + "\t") << "No expression to return." << std::endl;
            }
        }

        std::shared_ptr<ExprNode> getExpression() const {
            return expression; // Return the expression being returned
        }

        void setFunction(Function func) {
            this->func = std::move(func); // Set the function associated with the return statement
            type = func.return_type; // Set the type of the return statement to the function's return type
        }
        Function getFunction() const {
            return func; // Return the function associated with the return statement
        }
        void setExpression(std::shared_ptr<ExprNode> expr) {
            expression = std::move(expr); // Set the expression being returned
        }
    private:
        std::shared_ptr<ExprNode> expression; // Expression to return
        Function func;
};

class Pragram: public ASTNode {
    public:
        Pragram() = default;
        ~Pragram() = default;
        void walk(std::string prefix) override {
            std::cout << prettyPrint(prefix) << "Program Node :" << std::endl;
            for (const auto& global_var: global_vars) {
                global_var->walk(prefix + "\t"); // Walk each statement
            }
            for (const auto& func : functions) {
                func->walk(prefix + "\t"); // Walk each function
            }
        }
        void addGlobalVariable(std::shared_ptr<VariableDeclareNode> var) {
            global_vars.push_back(std::move(var)); // Add a new global variable
        }
        void addFunction(std::shared_ptr<FunctionDeclareNode> func) {
            functions.push_back(std::move(func)); // Add a new function
        }
        std::vector<std::shared_ptr<VariableDeclareNode>> getGlobalVariables() const {
            return global_vars; // Return the list of global variables
        }
        std::vector<std::shared_ptr<FunctionDeclareNode>> getFunctions() const {
            return functions; // Return the list of functions
        }
    private:
        std::vector<std::shared_ptr<VariableDeclareNode>> global_vars; // List of statements in the program
        std::vector<std::shared_ptr<FunctionDeclareNode>> functions; // List of functions in the program
};