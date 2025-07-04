#include "parser/parser.h"
#include "common/defs.h"
#include "assembly/backend/x86_64/x86_64.h" // Include the header defining X86AssemblyCode
// #include "assembly/backend/x86_64/X86AssemblyCode.h" // Ensure the complete definition of X86AssemblyCode is included
#include <iostream>
#include <vector>
#include <memory>
#pragma once

class GenCode {
    public:
        GenCode(std::string outputFileName) {
            assemblyCode = std::make_unique<X86AssemblyCode>(outputFileName);
        }
        ~GenCode() = default;

        // Generate code for the given AST node
        void generate(const std::shared_ptr<Pragram>& ast);
    private:
        std::unique_ptr<X86AssemblyCode> assemblyCode; // Pointer to AssemblyCode object to hold generated code
        
        Reg cgload(Value value) {
            return assemblyCode->cgload(value); // Load the value into a register
        }

        Reg cgadd(Reg reg1, Reg reg2) {
            return assemblyCode->cgadd(reg1, reg2); // Add the values in the registers and return the result
        }

        Reg cgsub(Reg reg1, Reg reg2) {
            return assemblyCode->cgsub(reg1, reg2);
        }

        Reg cgneg(Reg reg) {
            return assemblyCode->cgneg(reg);
        }

        Reg cgmul(Reg reg1, Reg reg2) {
            return assemblyCode->cgmul(reg1, reg2);
        }

        Reg cgdiv(Reg reg1, Reg reg2) {
            return assemblyCode->cgdiv(reg1, reg2);
        }

        void cgprintlong(Reg reg) {
            assemblyCode->cgprintlong(reg);
        }

        void cgpreamble() {
            assemblyCode->cgpreamble();
        }

        void cgpostamble() {
            assemblyCode->cgpostamble();
        }

        Reg cgloadsym(Symbol identifier, PrimitiveType type) {
            return assemblyCode->cgloadsym(identifier, type);
        }

        Reg cgstorsym(Reg r, Symbol identifier, PrimitiveType type) {
            return assemblyCode->cgstorsym(r, identifier, type);
        }

        void cgglobsym(Symbol sym, std::shared_ptr<ArrayInitializer> init = nullptr) {
            assemblyCode->cgglobsym(sym, init); // Generate global symbol definition
        }

        void freereg(Reg reg) {
            assemblyCode->freereg(reg);
        }

        Reg cgcompare(Reg r1, Reg r2, const char *op) {
            return assemblyCode->cgcompare(r1, r2, op);
        }

        Reg cgequal(Reg r1, Reg r2) {
            return assemblyCode->cgequal(r1, r2);
        }

        Reg cgnotequal(Reg r1, Reg r2) {
            return assemblyCode->cgnotequal(r1, r2);
        }

        Reg cggreaterthan(Reg r1, Reg r2) {
            return assemblyCode->cggreaterthan(r1, r2);
        }

        Reg cglessthan(Reg r1, Reg r2) {
            return assemblyCode->cglessthan(r1, r2);
        }

        Reg cggreaterequal(Reg r1, Reg r2) {
            return assemblyCode->cggreaterequal(r1, r2);
        }

        Reg cglessequal(Reg r1, Reg r2) {
            return assemblyCode->cglessequal(r1, r2);
        }

        Reg cgnot(Reg r1) {
            return assemblyCode->cgnot(r1);
        }

        void cglabel(const char *label) {
            assemblyCode->cglabel(label);
        }

        void cgjump(const char* label) {
            assemblyCode->cgjump(label);
        }

        void cgequaljump(Reg r1, Reg r2, const char *label) {
            assemblyCode->cgequaljump(r1, r2, label);
        }

        void cgnotequaljump(Reg r1, Reg r2, const char *label) {
            assemblyCode->cgnotequaljump(r1, r2, label);
        }

        void cggreaterequaljump(Reg r1, Reg r2, const char *label) {
            assemblyCode->cggreaterequaljump(r1, r2, label);
        }

        void cglessequaljump(Reg r1, Reg r2, const char *label) {
            assemblyCode->cglessequaljump(r1, r2, label);
        }

        void cglessthanjump(Reg r1, Reg r2, const char *label) {
            assemblyCode->cglessthanjump(r1, r2, label);
        }

        void cggreaterthanjump(Reg r1, Reg r2, const char *label) {
            assemblyCode->cggreaterthanjump(r1, r2, label);
        }

        void cgfuncpreamble(Function name) {
            assemblyCode->cgfuncpreamble(name);
        }

        void cgfuncpostamble(Function name, const char *label) {
            assemblyCode->cgfuncpostamble(name, label);
        }

        Reg cgint2char(Reg reg) {
            return assemblyCode->cgint2char(reg);
        }

        Reg cgchar2int(Reg reg) {
            return assemblyCode->cgchar2int(reg);
        }

        Reg cgfloat2int(Reg reg) {
            return assemblyCode->cgfloat2int(reg);
        }

        Reg cgint2float(Reg reg) {
            return assemblyCode->cgint2float(reg);
        }
        Reg cgfloat2char(Reg reg) {
            return assemblyCode->cgfloat2char(reg);
        }
        Reg cgchar2float(Reg reg) {
            return assemblyCode->cgchar2float(reg);
        }
        Reg cgint2long(Reg reg) {
            return assemblyCode->cgint2long(reg);
        }
        Reg cglong2int(Reg reg) {
            return assemblyCode->cglong2int(reg);
        }
        Reg cgfloat2long(Reg reg) {
            return assemblyCode->cgfloat2long(reg);
        }
        Reg cglong2float(Reg reg) {
            return assemblyCode->cglong2float(reg);
        }

        Reg cgchar2long(Reg reg) {
            return assemblyCode->cgchar2long(reg);
        }
        Reg cglong2char(Reg reg) {
            return assemblyCode->cglong2char(reg);
        }

        void cgprintfloat(Reg reg) {
            return assemblyCode->cgprintfloat(reg);
        }
        Reg cgcall(const char *name, PrimitiveType ret_type) {
            return assemblyCode->cgcall(name, ret_type); // Call the function with the specified name and return type
        }
        Reg cgaddress(Symbol identifier) {
            return assemblyCode->cgaddress(identifier);
        }
        Reg cgderef(Reg reg, PrimitiveType type) {
            return assemblyCode->cgderef(reg, type);
        }
        Reg cgshlconst(Reg reg, int value) {
            return assemblyCode->cgshlconst(reg, value);
        }
        void cglocalsym(Symbol sym) {
            assemblyCode->cglocalsym(sym);
        }
        Reg cgstorderef(Reg reg, Reg addr, PrimitiveType type) {
            return assemblyCode->cgstorderef(reg, addr, type);
        }
        void cginc(Symbol identifier, PrimitiveType type) {
            assemblyCode->cginc(identifier, type);
        }
        void cgdec(Symbol identifier, PrimitiveType type) {
            assemblyCode->cgdec(identifier, type);
        }
        void cginc(Reg addr, PrimitiveType type) {
            assemblyCode->cginc(addr, type);
        }
        void cgdec(Reg addr, PrimitiveType type) {
            assemblyCode->cgdec(addr, type);
        }
        Reg cginvert(Reg reg) {
            return assemblyCode->cginvert(reg);
        }
        Reg cgand(Reg reg1, Reg reg2) {
            return assemblyCode->cgand(reg1, reg2);
        }
        Reg cgor(Reg reg1, Reg reg2) {
            return assemblyCode->cgor(reg1, reg2);
        }
        Reg cgxor(Reg reg1, Reg reg2) {
            return assemblyCode->cgxor(reg1, reg2);
        }
        Reg cgshl(Reg reg1, Reg reg2) {
            return assemblyCode->cgshl(reg1, reg2);
        }
        Reg cgshr(Reg reg1, Reg reg2) {
            return assemblyCode->cgshr(reg1, reg2);
        }

        void cgresetparamcount() {
            assemblyCode->cgresetparamcount(); // Reset the parameter count for the function
        }
        Reg cgparamaddr(Symbol identifier) {
            return assemblyCode->cgparamaddr(identifier); // Get the address of the parameter in the function
        }

        void cgadjuststack(int size) {
            assemblyCode->cgadjuststack(size); // Adjust the stack size for function calls
        }

        void cgloadparamtostack(Reg reg) {
            assemblyCode->cgloadparamtostack(reg); // Load the parameter into the stack
        }

        void cgloadparamtoreg(Reg reg, int idx) {
            assemblyCode->cgloadparamtoreg(reg, idx); // Load the parameter into the register
        }
        void cglocalarrayzeroinit(int base, int left_size, int current_size, int elem_size) {
            assemblyCode->cglocalarrayzeroinit(base, left_size, current_size, elem_size); // Initialize local array to zero
        }
        void cgrestorescene(const std::vector<Reg> &protected_regs) {
            assemblyCode->cgrestorescene(protected_regs); // Restore the protected registers from the stack
        }
        std::vector<Reg> cgprotectscene() {
            return assemblyCode->cgprotectscene(); // Protect the scene before generating code for the function call
        }
        Reg cgmod(Reg reg1, Reg reg2) {
            return assemblyCode->cgmod(reg1, reg2); // Generate code for modulo operation
        }
        Reg walkPragram(const std::shared_ptr<Pragram>& ast);
        Reg walkStatement(const std::shared_ptr<StatementNode>& ast);
        Reg walkExpr(const std::shared_ptr<ExprNode>& ast);
        void walkCondition(const std::shared_ptr<ExprNode>& ast, std::string false_label);
        void walkFunction(const std::shared_ptr<FunctionDeclareNode>& ast);
        Reg walkFunctionCall(const std::shared_ptr<FunctionCallNode>& ast);
        void walkReturn(const std::shared_ptr<ReturnStatementNode>& ast);
        Reg transformType(PrimitiveType type, PrimitiveType target_type, Reg reg);
        void localArrayInit(const std::shared_ptr<ArrayInitializer>& init);
        void walkFunctionParam(const std::shared_ptr<FunctionParamNode>& ast);
};