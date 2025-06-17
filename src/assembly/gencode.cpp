#include "assembly/gencode.h"

Reg GenCode::transformType(PrimitiveType from, PrimitiveType to, Reg reg) {
    if (from == P_INT && to == P_CHAR) {
        return cgint2char(reg); // Transform Reg to char
    } else if (from == P_CHAR && to == P_INT) {
        return cgchar2int(reg);
    } else if (from == P_INT && to == P_FLOAT) {
        return cgint2float(reg); // Transform Reg to float
    } else if (from == P_FLOAT && to == P_INT) {
        return cgfloat2int(reg);
    } else if (from == P_FLOAT && to == P_CHAR) {
        return cgfloat2char(reg);
    } else if (from == P_CHAR && to == P_FLOAT) {
        return cgchar2float(reg);
    } else if (from == P_LONG && to == P_INT) {
        return cglong2int(reg);
    } else if (from == P_INT && to == P_LONG) {
        return cgint2long(reg);
    } else if (from == P_FLOAT && to == P_LONG) {
        return cgfloat2long(reg);
    } else if (from == P_LONG && to == P_FLOAT) {
        return cglong2float(reg); // Convert long to int, then to float
    } else if (from == P_CHAR && to == P_LONG) {
        return cgchar2long(reg); // Convert char to int, then to long
    } else if (from == P_LONG && to == P_CHAR) {
        return cglong2char(reg); // Convert long to int, then to char
    } else {
        throw std::runtime_error("GenCode::transformType: Unsupported type transformation");
    }
}

Reg GenCode::walkExpr(const std::shared_ptr<ExprNode>& ast) { 
    if (auto x = std::dynamic_pointer_cast<BinaryExpNode>(ast)) {
        // TODO
        // if (x->getOp() == A_AND) {
        //     return walkAndExpr(x);
        // } else if (x->getOp() == A_OR) {
        //     return walkOrExpr(x);
        // }
        // Handle binary expression node
        Reg reg1 = walkExpr(x->getLeft());
        if (x->getLeft()->isNeedTransform()) {
           reg1 = transformType(x->getLeft()->getType(), x->getCalType(), reg1);
        }
        Reg reg2 = walkExpr(x->getRight());
        if (x->getRight()->isNeedTransform()) {
           reg2 = transformType(x->getRight()->getType(), x->getCalType(), reg2);
        }
        
        Reg ret;
        switch (x->getOp()) {
            case A_ADD: return cgadd(reg1, reg2); // Add the two registers and return the result
            case A_SUBTRACT: return cgsub(reg1, reg2); // Subtract the two registers and return the result
            case A_MULTIPLY: return cgmul(reg1, reg2); // Multiply the two registers and return the result
            case A_DIVIDE: return cgdiv(reg1, reg2); // Divide the two registers and return the result
            case A_EQ: ret = cgequal(reg1, reg2); break;
            case A_NE: ret = cgnotequal(reg1, reg2); break;
            case A_LT: ret = cglessthan(reg1, reg2); break;
            case A_LE: ret = cglessequal(reg1, reg2); break;
            case A_GT: ret = cggreaterthan(reg1, reg2); break;
            case A_GE: ret = cggreaterequal(reg1, reg2); break;
            default:
                throw std::runtime_error("GenCode::generate: Unknown binary expression type");
        }
        ret.type = P_LONG;
        return ret;
    } else if (auto x = std::dynamic_pointer_cast<UnaryExpNode>(ast)) {
        Reg reg = walkExpr(x->getExpr()); // Walk the expression in the unary node
        // Handle unary expression node
        if (x->getOp() == U_MINUS) {
            return cgneg(reg); // Load the integer value into a register
        } else if (x->getOp() == U_PLUS) {
            return reg;
        } else if (x->getOp() == U_NOT) {
            return cgnot(reg);
        } else  {
            throw std::runtime_error("GenCode::generate: Unknown unary expression type");
        }
    } else if (auto x = std::dynamic_pointer_cast<ValueNode>(ast)) {
        // Handle value node
        return cgload(x->getValue()); // Load the value into a register
    } else if (auto x = std::dynamic_pointer_cast<LValueNode>(ast)) {
        return cgloadglob(x->getIdentifier().name.c_str(), x->getType()); // Load the global variable into a register
    } else if (auto x = std::dynamic_pointer_cast<AssignmentNode>(ast)) {
        // Handle assignment node
        Reg reg = walkExpr(x->getExpr()); // Walk the expression in the assignment node
        if (x->isNeedTransform()) {
            reg = transformType(x->getExpr()->getType(), x->getIdentifier().type, reg); // Transform the type if needed
        }
        cgstorglob(reg, x->getIdentifier().name.c_str(), x->getIdentifier().type); // Store the value in the global variable
        return reg; // Return the register containing the result

    } else if (auto x = std::dynamic_pointer_cast<FunctionCallNode>(ast)) {
        return walkFunctionCall(x);
    } else {
        throw std::runtime_error("GenCode::generate: Unknown expression node type");
    }
}

void GenCode::walkCondition(const std::shared_ptr<ExprNode>& ast, std::string false_label) {
    if (auto x = std::dynamic_pointer_cast<BinaryExpNode>(ast)) {
        Reg reg1 = walkExpr(x->getLeft()); // Walk the left expression
        if (x->getLeft()->isNeedTransform()) {
            reg1 = transformType(x->getLeft()->getType(), x->getType(), reg1);
         }
        Reg reg2 = walkExpr(x->getRight()); // Walk the right expression
        if (x->getRight()->isNeedTransform()) {
            reg2 = transformType(x->getRight()->getType(), x->getType(), reg2);
         }
        switch (x->getOp()) {
            case A_EQ: return cgnotequaljump(reg1, reg2, false_label.c_str());
            case A_NE: return cgequaljump(reg1, reg2, false_label.c_str());
            case A_LT: return cggreaterequaljump(reg1, reg2, false_label.c_str());
            case A_LE: return cggreaterthanjump(reg1, reg2, false_label.c_str());
            case A_GT: return cglessequaljump(reg1, reg2, false_label.c_str());
            case A_GE: return cglessthanjump(reg1, reg2, false_label.c_str());
            case A_ADD: reg1 = cgadd(reg1, reg2); break;// Add the two registers and return the result
            case A_SUBTRACT: reg1 = cgsub(reg1, reg2); break;// Subtract the two registers and return the result
            case A_MULTIPLY: reg1 = cgmul(reg1, reg2); break;// Multiply the two registers and return the result
            case A_DIVIDE: reg1 = cgdiv(reg1, reg2); break;// Divide the two registers and return the result
            default:
                throw std::runtime_error("GenCode::walkCondition: Unknown binary expression type");
        }
        reg2 = cgload(Value{ .type = P_INT, .ivalue = 0 }); // Load zero into a register
        return cgequaljump(reg1, reg2, false_label.c_str()); // Compare the result with zero and jump if equal
    } else {
        Reg reg1 = walkExpr(ast); // Walk the expression in the condition
        Reg reg2 = cgload(Value{ .type = P_LONG, .ivalue = 0 }); // Load zero into a register
        return cgequaljump(reg1, reg2, false_label.c_str()); // Compare the result with zero and jump if equal
    }
}

Reg GenCode::walkStatement(const std::shared_ptr<StatementNode>& ast) {
    if (auto x = std::dynamic_pointer_cast<BlockNode>(ast)) {
        std::string block_label = labelAllocator.getLabel(LableType::BLOCK_LABEL);
        cglabel(block_label.c_str()); // Generate a label for the block
        auto stmts = x->getStatements();
        for (const auto& stmt : stmts) {
            walkStatement(stmt); // Walk each statement in the statements node
        }
        return Reg{.type = P_NONE, .idx = 0};
    } else if (auto x  = std::dynamic_pointer_cast<PrintStatementNode>(ast)) {
        Reg reg = walkExpr(x->getExpression());
        if (x->isNeedTransform()) {
            reg = transformType(x->getExpression()->getType(), x->getType(), reg); // Transform the type if needed
        }
        if (x->getType() == P_LONG) cgprintlong(reg);
        else cgprintfloat(reg); // Print the value in the register
        return Reg{.type = P_NONE, .idx = 0};
    } else if (auto x = std::dynamic_pointer_cast<VariableDeclareNode>(ast)) {
        for (const auto& identifier: x->getIdentifiers()) {
            Symbol sym = symbol_table.getSymbol(identifier); // Get the symbol from the symbol table
            cgglobsym(sym); // Declare a global variable with the given identifier and type
            auto initializer = x->getInitializer(identifier);
            if (initializer != nullptr) {
                Reg reg = walkExpr(initializer);
                if (initializer->isNeedTransform()) {
                    reg = transformType(initializer->getType(), x->getVariableType(), reg); // Transform the type if needed
                }
                cgstorglob(reg, identifier.c_str(), x->getVariableType()); // Store the value in the global variable
                assemblyCode->freereg(reg);
            }
        }
        return Reg{.type = P_NONE, .idx = 0};
    } else if (auto x = std::dynamic_pointer_cast<IfStatementNode>(ast)) {
        // 目前只考虑都是Block
        // TODO: assignment寄存器的释放
        std::string if_label_no = labelAllocator.getLabel(LableType::IF_LABEL);
        std::string if_true = "IF_TRUE_" + if_label_no;
        std::string if_false = "IF_FALSE_" + if_label_no;
        std::string if_end = "IF_END_" + if_label_no;
        walkCondition(x->getCondition(), if_false); // Walk the condition and generate code for the jump
        cglabel(if_true.c_str());
        walkStatement(x->getThenStatement()); // Walk the then statement
        cgjump(if_end.c_str());
        cglabel(if_false.c_str());
        if (x->getElseStatement() != nullptr) {
            walkStatement(x->getElseStatement()); // Walk the else statement
        }
        cglabel(if_end.c_str()); // Generate the end label for the if statement
        return Reg{.type = P_NONE, .idx = 0};
    } else if (auto x = std::dynamic_pointer_cast<WhileStatementNode>(ast)) {
        std::string while_label_no = labelAllocator.getLabel(LableType::WHILE_LABEL);
        std::string while_start = "WHILE_START_" + while_label_no;
        std::string while_end = "WHILE_END_" + while_label_no;
        cglabel(while_start.c_str()); // Generate the start label for the while loop
        walkCondition(x->getCondition(), while_end); // Walk the condition and generate code for the jump
        walkStatement(x->getBody()); // Walk the body of the while loop
        cgjump(while_start.c_str()); // Jump back to the start of the loop
        cglabel(while_end.c_str()); // Generate the end label for the while loop
        return Reg{.type = P_NONE, .idx = 0};
    } else if (auto x = std::dynamic_pointer_cast<ForStatementNode>(ast)) {
        std::string for_label_no = labelAllocator.getLabel(LableType::FOR_LABEL);
        std::string for_start = "FOR_START_" + for_label_no;
        std::string for_end = "FOR_END_" + for_label_no;
        if (x->getPreopStatement() != nullptr) {
            walkStatement(x->getPreopStatement()); // Walk the pre-operation statement
        }
        cglabel(for_start.c_str()); // Generate the start label for the for loop
        walkCondition(x->getCondition(), for_end); // Walk the condition and generate code for the jump
        walkStatement(x->getBody()); // Walk the body of the for loop
        if (x->getPostopStatement() != nullptr) {
            walkStatement(x->getPostopStatement()); // Walk the post-operation statement
        }
        cgjump(for_start.c_str()); // Jump back to the start of the loop
        cglabel(for_end.c_str()); // Generate the end label for the for loop
        return Reg{.type = P_NONE, .idx = 0};
    } else if (auto x = std::dynamic_pointer_cast<ExprNode>(ast)) {
        // Handle expression node
        Reg reg = walkExpr(x); // Walk the expression node to generate code
        assemblyCode->freereg(reg); // Free the register after use
        return Reg{.type = P_NONE, .idx = 0};
    } else if (auto x = std::dynamic_pointer_cast<ReturnStatementNode>(ast)) {
        walkReturn(x);
        return Reg{.type = P_NONE, .idx = 0};
    } else {
        throw std::runtime_error("GenCode::generate: Unknown statement node type");
    }
}

void GenCode::walkFunction(const std::shared_ptr<FunctionDeclareNode>& ast) {
    std::string func_name = ast->getIdentifier();
    cgfuncpreamble(func_name.c_str()); // Generate function preamble code
    walkStatement(ast->getBody()); // Walk the function body to generate code
    cgfuncpostamble((func_name + "_end").c_str()); // Generate function postamble code
}

Reg GenCode::walkPragram(const std::shared_ptr<Pragram>& ast) {
    for (const auto &x: ast->getGlobalVariables()) {
        walkStatement(x); // Walk each global variable declaration to generate code
    } 
    for (const auto &x: ast->getFunctions()) {
        walkFunction(x); // Walk each function to generate code
    }
    return Reg{.type = P_NONE, .idx = 0}; // Return a dummy register for now
}

void GenCode::generate(const std::shared_ptr<Pragram>& ast) {
    cgpreamble(); // Generate preamble code
    assert(walkPragram(ast).type == P_NONE); // Walk the AST to generate code
    // cgpostamble(); // Generate postamble code
}

// 目前只考虑无参数或者一个参数调用
Reg GenCode::walkFunctionCall(const std::shared_ptr<FunctionCallNode>& ast) {
    auto args = ast->getArguments();
    if (args.size() > 1) {
        throw std::runtime_error("GenCode::walkFunctionCall: Function calls with more than one argument are not supported yet");
    }
    Function func = symbol_table.getFunction(ast->getIdentifier()); // Get the function from the symbol table
    Reg reg = Reg{.type = func.return_type, .idx = -1};
    if (args.size() == 1) {
        // 暂时也不考虑类型转换
        reg = walkExpr(args[0]); // Walk the argument expression to generate code
    } 
    reg = cgcall(ast->getIdentifier().c_str(), reg); // Call the function with the argument

    return reg; // Return a dummy register for now
    // return cgcall(ast->getIdentifier().name.c_str(), args); // Call the function with the arguments
}

void GenCode::walkReturn(const std::shared_ptr<ReturnStatementNode>& ast) {
    const char* end_label = (ast->getFunction().name + "_end").c_str();
    if (ast->getExpression() != nullptr) {
        Reg reg = walkExpr(ast->getExpression()); // Walk the return expression to generate code
        if (ast->isNeedTransform()) {
            reg = transformType(ast->getExpression()->getType(), ast->getType(), reg); // Transform the type if needed
        }
        assemblyCode->cgreturn(reg, end_label); // Generate return code with the register
    } else {
        assemblyCode->cgreturn(Reg{.type = P_VOID, .idx = 0}, end_label); // Generate return code without a value
    }
}