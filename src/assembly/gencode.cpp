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
        Reg reg2 = walkExpr(x->getRight());
        // assert(reg1.type == reg2.type); // Ensure both registers have the same type
        Reg ret;
        switch (x->getOp()) {
            case A_ADD: return cgadd(reg1, reg2); // Add the two registers and return the result
            case A_SUBTRACT: return cgsub(reg1, reg2); // Subtract the two registers and return the result
            case A_MULTIPLY: return cgmul(reg1, reg2); // Multiply the two registers and return the result
            case A_DIVIDE: return cgdiv(reg1, reg2); // Divide the two registers and return the result
            case A_MOD: return cgmod(reg1, reg2); 
            case A_EQ: ret = cgequal(reg1, reg2); break;
            case A_NE: ret = cgnotequal(reg1, reg2); break;
            case A_LT: ret = cglessthan(reg1, reg2); break;
            case A_LE: ret = cglessequal(reg1, reg2); break;
            case A_GT: ret = cggreaterthan(reg1, reg2); break;
            case A_GE: ret = cggreaterequal(reg1, reg2); break;
            case A_AND: ret = cgand(reg1, reg2); break; // Perform bitwise AND operation
            case A_OR: ret = cgor(reg1, reg2); break; // Perform bitwise OR operation
            case A_XOR: ret = cgxor(reg1, reg2); break; //
            case A_LSHIFT: ret = cgshl(reg1, reg2); break; // Perform left shift operation
            case A_RSHIFT: ret = cgshr(reg1, reg2); break;
            default:
                throw std::runtime_error("GenCode::generate: Unknown binary expression type");
        }
        ret.type = P_LONG;
        return ret;
    } else if (auto x = std::dynamic_pointer_cast<UnaryExpNode>(ast)) {
        if (x->getOp() == U_ADDR || x->getOp() == U_DEREF) {
            auto y = std::dynamic_pointer_cast<LValueNode>(x->getExpr());
            if (y != nullptr && !y->isArray()) {
                if (x->getOp() == U_ADDR) {
                    return cgaddress(y->getIdentifier()); // Get the address of the identifier
                } else if (x->getOp() == U_DEREF) {
                    Reg reg = cgloadsym(y->getIdentifier(), y->getCalculateType()); // Load the global variable into a register
                    return cgderef(reg, y->getPrimitiveType()); // Dereference the pointer to get the value
                }
            } else {
                Reg base = walkExpr(x->getExpr()); // For array, just walk the expression, 得到数据指针 
                if (x->getOp() == U_ADDR) return base;
                else if (x->getOp() == U_DEREF) {
                    // Dereference the pointer to get the value
                    return cgderef(base, x->getExpr()->getPrimitiveType());
                }
            }
        }
        if (x->getOp() == U_PREINC || x->getOp() == U_PREDEC) {
            if (auto y = std::dynamic_pointer_cast<LValueNode>(x->getExpr())) {
                if (x->getOp() == U_PREINC) {
                    cginc(y->getIdentifier(), y->getIdentifier().type); // Increment the register
                } else if (x->getOp() == U_PREDEC) {
                    cgdec(y->getIdentifier(), y->getIdentifier().type); // Decrement the register
                }
            } else if (auto y = std::dynamic_pointer_cast<UnaryExpNode>(x->getExpr())) {
                assert(y->getOp() == U_DEREF); // Ensure the unary operation is dereference
                Reg addr = walkExpr(y->getExpr()); // Walk the expression in the unary node
                if (x->getOp() == U_PREINC) {
                    cginc(addr, x->getPrimitiveType()); // Increment the address register
                } else if (x->getOp() == U_PREDEC) {
                    cgdec(addr, x->getPrimitiveType()); // Decrement the address register
                }
            }
        }
    
        Reg reg = walkExpr(x->getExpr()); // Walk the expression in the unary node

        if (x->getOp() == U_POSTINC || x->getOp() == U_POSTDEC) {
            if (auto y = std::dynamic_pointer_cast<LValueNode>(x->getExpr())) {
                if (x->getOp() == U_POSTINC) {
                    cginc(y->getIdentifier(), y->getIdentifier().type); // Increment the register
                } else if (x->getOp() == U_POSTDEC) {
                    cgdec(y->getIdentifier(), y->getIdentifier().type); // Decrement the register
                }
            } else if (auto y = std::dynamic_pointer_cast<UnaryExpNode>(x->getExpr())) {
                assert(y->getOp() == U_DEREF); // Ensure the unary operation is dereference
                Reg addr = walkExpr(y->getExpr()); // Walk the expression in the unary node
                if (x->getOp() == U_POSTINC) {
                    cginc(addr, x->getPrimitiveType()); // Increment the address register
                } else if (x->getOp() == U_POSTDEC) {
                    cgdec(addr, x->getPrimitiveType()); // Decrement the address register
                }
            }
        }
        // Handle unary expression node
        if (x->getOp() == U_MINUS) {
            return cgneg(reg); // Load the integer value into a register
        } else if (x->getOp() == U_PLUS) {
            return reg;
        } else if (x->getOp() == U_NOT) {
            return cgnot(reg);
        } else if (x->getOp() == U_TRANSFORM) {
            return transformType(x->getExpr()->getPrimitiveType(), x->getPrimitiveType(), reg); // Transform the type of the expression
        } else if (x->getOp() == U_SCALE) {
            Reg leftreg = reg;
            if (leftreg.type != P_LONG) {
                leftreg = transformType(leftreg.type, P_LONG, leftreg); // Ensure the left register is treated as a long integer
            }
            Reg rightreg;
            switch (x->getOffset()) {
                case 2: return(cgshlconst(leftreg, 1));
                case 4: return(cgshlconst(leftreg, 2));
                case 8: return(cgshlconst(leftreg, 3));
                default:
                  // Load a register with the size and
                  // multiply the leftreg by this size
                      rightreg= cgload(Value{.type = P_LONG, .ivalue = x->getOffset()});
                      return (cgmul(leftreg, rightreg));
            }
        } else if (x->getOp() == U_INVERT) {
            return cginvert(reg); // Perform bitwise NOT operation
        }
        return reg; // Return the register containing the result
    } else if (auto x = std::dynamic_pointer_cast<ValueNode>(ast)) {
        // Handle value node
        return cgload(x->getValue()); // Load the value into a register
    } else if (auto x = std::dynamic_pointer_cast<LValueNode>(ast)) {
        // 在这一步，指针类型会被转换为P_LONG寄存器
        if (!x->isArray()) return cgloadsym(x->getIdentifier(), x->getCalculateType()); // Load the global variable into a register
        else {
            Reg index;
            if (x->getIndex() != nullptr){
                index = walkExpr(x->getIndex()); // Walk the index expression
                index.type = P_LONG; // Ensure the index is treated as a long integer
            }
            Reg base;
            if (x->isParam()) base = cgloadsym(x->getIdentifier(), x->getCalculateType());
            else base = cgaddress(x->getIdentifier()); // Load the base address of the array into a register
            if (x->getIndex() != nullptr) {
                return cgadd(base, index);
            }
            return base;
        }
    } else if (auto x = std::dynamic_pointer_cast<AssignmentNode>(ast)) {
        // Handle assignment node
        Reg reg = walkExpr(x->getExpr()); // Walk the expression in the assignment node

        if (auto y = std::dynamic_pointer_cast<LValueNode>(x->getLvalue())) {
            cgstorsym(reg, y->getIdentifier(), x->getCalculateType());
        } else if (auto y = std::dynamic_pointer_cast<UnaryExpNode>(x->getLvalue())) {
            assert(y->getOp() == U_DEREF); // Ensure the unary operation is dereference
            Reg addr = walkExpr(y->getExpr()); // Walk the expression in the unary node
            cgstorderef(reg, addr, x->getPrimitiveType());
        }
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
        Reg reg2 = walkExpr(x->getRight()); // Walk the right expression
        assert(reg1.type == reg2.type); // Ensure both registers have the same type
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
            case A_AND: reg1 = cgand(reg1, reg2); break; // Perform bitwise AND operation
            case A_OR: reg1 = cgor(reg1, reg2); break; //
            case A_XOR: reg1 = cgxor(reg1, reg2); break; //
            case A_LSHIFT: reg1 = cgshl(reg1, reg2); break;
            case A_RSHIFT: reg1 = cgshr(reg1, reg2); break; 
            default:
                throw std::runtime_error("GenCode::walkCondition: Unknown binary expression type");
        }
        reg2 = cgload(Value{ .type = P_LONG, .ivalue = 0 }); // Load zero into a register
        return cgequaljump(reg1, reg2, false_label.c_str()); // Compare the result with zero and jump if equal
    } else {
        Reg reg1 = walkExpr(ast); // Walk the expression in the condition
        reg1.type = P_LONG; // Ensure the register is treated as a long integer
        Reg reg2 = cgload(Value{ .type = P_LONG, .ivalue = 0 }); // Load zero into a register
        return cgequaljump(reg1, reg2, false_label.c_str()); // Compare the result with zero and jump if equal
    }
}

void GenCode::localArrayInit(const std::shared_ptr<ArrayInitializer>& y) {
    y->getValuePos();
    PrimitiveType type = y->getPrimitiveType();
    auto exprs = y->getElements();
    auto pos = y->getPosOnStack();
    for (size_t i = 0; i < exprs.size(); i++) {
        if (auto z = std::dynamic_pointer_cast<ValueNode>(exprs[i])) {
            Symbol sym = {"", type, 5, false, false, pos[i]};
            Reg reg;
            if (type == P_INT) {
                reg = cgload(Value{.type = P_INT, .ivalue = z->getIntValue()}); // Load the integer value into a register
            } else if (type == P_FLOAT) {
                reg = cgload(Value{.type = P_FLOAT, .fvalue = z->getFloatValue()}); // Load the float value into a register
            } else if (type == P_LONG) {
                reg = cgload(Value{.type = P_LONG, .lvalue = z->getLongValue()}); // Load the long value into a register
            } else if (type == P_CHAR) {
                reg = cgload(Value{.type = P_CHAR, .ivalue = z->getCharValue()}); // Load the char value into a register
            } else {
                throw std::runtime_error("GenCode::walkStatement: Unknown array element type");
            }
            cgstorsym(reg, sym, type); // Store the value in the global variable
            assemblyCode->freereg(reg); // Free the register after use
        } else if (auto z = std::dynamic_pointer_cast<ArrayInitializer>(exprs[i])) {
            localArrayInit(z); 
        }
    }
    if (y->getLeftSize()) {
        auto sym = y->getSymbol();
        cglocalarrayzeroinit(y->getBaseOffset(), y->getLeftSize(), y->getCurrentSize(), symbol_table.typeToSize(type));
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

        if (x->getCalculateType() == P_LONG) cgprintlong(reg);
        else cgprintfloat(reg); // Print the value in the register
        return Reg{.type = P_NONE, .idx = 0};
    } else if (auto x = std::dynamic_pointer_cast<VariableDeclareNode>(ast)) {
        for (const auto& identifier: x->getIdentifiers()) {
            
            cglocalsym(identifier); // Declare a global variable with the given identifier and type
            auto initializer = x->getInitializer(identifier);
            if (initializer == nullptr) continue;;
            if (!identifier.is_array) {
                Reg reg = walkExpr(initializer);
                cgstorsym(reg, identifier, x->getVariableType()); // Store the value in the global variable
                assemblyCode->freereg(reg);
            } else {
                if (auto y = std::dynamic_pointer_cast<ArrayInitializer>(initializer)) {
                    y->setBaseOffset();
                    localArrayInit(y); // Initialize the local array variable
                } else {
                    throw std::runtime_error("GenCode::walkStatement: Array initializer expected");
                }
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
        std::string while_start = x->getWhileStartLabel(); // Get the start label for the while loop
        std::string while_end = x->getWhileEndLabel(); // Get the end label for the while loop
        cglabel(while_start.c_str()); // Generate the start label for the while loop
        walkCondition(x->getCondition(), while_end); // Walk the condition and generate code for the jump
        walkStatement(x->getBody()); // Walk the body of the while loop
        cgjump(while_start.c_str()); // Jump back to the start of the loop
        cglabel(while_end.c_str()); // Generate the end label for the while loop
        return Reg{.type = P_NONE, .idx = 0};
    } else if (auto x = std::dynamic_pointer_cast<ForStatementNode>(ast)) {
        std::string for_start = x->getForStartLabel(); // Get the start label for the for loop
        std::string for_end = x->getForEndLabel(); // Get the end label for the for loop
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
        if (reg.type != P_NONE) assemblyCode->freereg(reg); // Free the register after use
        return Reg{.type = P_NONE, .idx = 0};
    } else if (auto x = std::dynamic_pointer_cast<ReturnStatementNode>(ast)) {
        walkReturn(x);
        return Reg{.type = P_NONE, .idx = 0};
    } else if (auto x = std::dynamic_pointer_cast<BreakStatementNode>(ast)) {
        cgjump(x->getLabel().c_str()); // Jump to the break label
        return Reg{.type = P_NONE, .idx = 0};
    } else if (auto x = std::dynamic_pointer_cast<ContinueStatementNode>(ast)) {
        cgjump(x->getLabel().c_str());
        return Reg{.type = P_NONE, .idx = 0}; // Jump to the continue label
    } else {
        throw std::runtime_error("GenCode::generate: Unknown statement node type");
    }
}

void GenCode::walkFunction(const std::shared_ptr<FunctionDeclareNode>& ast) {
    walkStatement(ast->getBody()); // Walk the function body to generate code
}

void GenCode::walkFunctionParam(const std::shared_ptr<FunctionParamNode>& ast) {
    cgresetparamcount(); // Reset the parameter count for the function
    for (const auto &identifier: ast->getParams()) {
        Reg reg = cgparamaddr(*identifier);
        if (reg.type == P_NONE) continue;
        cgstorsym(reg, *identifier, reg.type); // Store the parameter address in the symbol table
        // assemblyCode->freereg(reg); // Free the register after use
    }
}

Reg GenCode::walkPragram(const std::shared_ptr<Pragram>& ast) {
    for (const auto &x: ast->getGlobalVariables()) {
        for (const auto &identifier: x->getIdentifiers()) {
            if (identifier.is_array) cgglobsym(identifier, std::dynamic_pointer_cast<ArrayInitializer> (x->getInitializer(identifier))); // Declare a global array variable with the given identifier and type
            else cgglobsym(identifier); // Declare a global variable with the given identifier and type
        }
    }
    // for (const auto &x: ast->getGlobalVariables()) {
    //     walkStatement(x); // Walk each global variable declaration to generate code
    // } 
    for (const auto &x: ast->getFunctions()) {
        std::string func_name = x->getIdentifier() ;
        Function func = symbol_table.getFunction(func_name); // Get the function from the symbol table
        cgfuncpreamble(func); // Generate function preamble code
        walkFunctionParam(x->getParams()); // Walk the function parameters to generate code
        if (func_name == "main") {
            // 全局变量初始化
            for (const auto &x: ast->getGlobalVariables()) {
                for (const auto &identifier: x->getIdentifiers()) {
                    auto initializer = x->getInitializer(identifier);
                    if (initializer != nullptr && !identifier.is_array) {
                        Reg reg = walkExpr(initializer);
                        reg = cgstorsym(reg, identifier, x->getVariableType()); // Store the value in the global variable
                        assemblyCode->freereg(reg);
                    }
                }
            }
        }
        walkFunction(x); // Walk each function to generate code
        cgfuncpostamble(func, (func_name + "_end").c_str()); // Generate function postamble code
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

    std::vector<Reg> used = cgprotectscene(); // Protect the scene before generating code for the function call

    auto args = ast->getArguments();
    int stack_offset = 0;
    if (ast->needAdjustStack()) {
        assemblyCode->cgadjuststack(8); // Adjust the stack size for the function call
        stack_offset = 8; // Set the stack offset for the function call
    } 
    std::vector<Reg> int_need_load_to_stack;
    std::vector<Reg> float_need_load_to_stack;
    int int_param_count = 0;
    int float_param_count = 0;

    for (const auto& arg : args) {
        PrimitiveType type = arg->getCalculateType();
        Reg reg = walkExpr(arg); // Walk the argument expression to generate code
        
        if (type == P_FLOAT) {
            if (float_param_count >= 8) {
                float_need_load_to_stack.push_back(reg);
            } else {
                cgloadparamtoreg(reg, float_param_count); // Load the float parameter to a register
            }
            float_param_count++;
        } else {
            if (int_param_count >= 6) {
                int_need_load_to_stack.push_back(reg);
            } else {
                cgloadparamtoreg(reg, int_param_count); // Load the integer parameter to a register
            }
            int_param_count++;
        }
    }

    for (auto reg = int_need_load_to_stack.rbegin(); reg != int_need_load_to_stack.rend(); reg++) {
        assemblyCode->cgloadparamtostack(*reg); // Load the integer parameter to the stack
        stack_offset += 8; // Increment the stack offset for each integer parameter
    }

    for (auto reg = float_need_load_to_stack.rbegin(); reg != float_need_load_to_stack.rend(); reg++) {
        assemblyCode->cgloadparamtostack(*reg); // Load the float parameter to the stack
        stack_offset += 8; // Increment the stack offset for each float parameter
    }

    Function func = symbol_table.getFunction(ast->getIdentifier()); // Get the function from the symbol table
    Reg reg = cgcall(ast->getIdentifier().c_str(), is_pointer(func.return_type)? P_LONG : func.return_type); // Call the function with the argument

    if (stack_offset) cgadjuststack(-stack_offset);
    cgrestorescene(used); // Restore the scene after generating code for the function call
    return reg;
}

void GenCode::walkReturn(const std::shared_ptr<ReturnStatementNode>& ast) {
    std::string end_label = ast->getFunction().name + "_end";
    if (ast->getExpression() != nullptr) {
        Reg reg = walkExpr(ast->getExpression()); // Walk the return expression to generate code
        assemblyCode->cgreturn(reg, end_label.c_str()); // Generate return code with the register
    } else {
        assemblyCode->cgreturn(Reg{.type = P_VOID, .idx = 0}, end_label.c_str()); // Generate return code without a value
    }
}