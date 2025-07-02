#include "common/defs.h"
#pragma once

class RegisterManager {
public:
    RegisterManager() = default;
    virtual Reg allocateRegister(PrimitiveType) = 0;
    virtual void freeAllRegister() = 0;
    virtual void freeRegister(Reg reg) = 0;
    virtual std::string getRegister(Reg reg) const = 0;
    ~RegisterManager() = default;
};


class AssemblyCode {
public:
    AssemblyCode() = default;
    ~AssemblyCode() = default;
    virtual Reg cgload(Value value) = 0;
    virtual Reg cgadd(Reg reg1, Reg reg2) = 0;
    virtual Reg cgsub(Reg reg1, Reg reg2) = 0;
    virtual Reg cgneg(Reg reg) = 0;
    virtual Reg cgmul(Reg reg1, Reg reg2) = 0;
    virtual Reg cgdiv(Reg reg1, Reg reg2) = 0;
    virtual void cgprintlong(Reg reg) = 0;
    virtual void cgprintfloat(Reg reg) = 0;
    virtual void cgpreamble() = 0;
    virtual void cgpostamble() = 0;
    virtual Reg cgloadsym(Symbol identifier, PrimitiveType type) = 0;
    virtual Reg cgstorsym(Reg r, Symbol identifier, PrimitiveType type) = 0;
    virtual void cglocalsym(Symbol sym) = 0;
    virtual void cgglobsym(Symbol sym, std::shared_ptr<ArrayInitializer> init = nullptr) = 0;
    virtual void freereg(Reg reg) = 0;
    virtual Reg cgcompare(Reg r1, Reg r2, const char *op) = 0;
    virtual Reg cgequal(Reg r1, Reg r2) = 0;
    virtual Reg cgnotequal(Reg r1, Reg r2) = 0;
    virtual Reg cggreaterthan(Reg r1, Reg r2) = 0;
    virtual Reg cglessthan(Reg r1, Reg r2) = 0;
    virtual Reg cggreaterequal(Reg r1, Reg r2) = 0;
    virtual Reg cglessequal(Reg r1, Reg r2) = 0;
    virtual Reg cgnot(Reg r1) = 0;
    virtual void cglabel(const char *label) = 0;
    virtual void cgjump(const char* label) = 0;
    virtual void cgequaljump(Reg r1, Reg r2, const char *label) = 0;
    virtual void cgnotequaljump(Reg r1, Reg r2, const char *label) = 0;
    virtual void cggreaterequaljump(Reg r1, Reg r2, const char *label) = 0;
    virtual void cglessequaljump(Reg r1, Reg r2, const char *label) = 0;
    virtual void cglessthanjump(Reg r1, Reg r2, const char *label) = 0;
    virtual void cggreaterthanjump(Reg r1, Reg r2, const char *label) = 0;
    virtual void cgfuncpreamble(Function func) = 0;
    virtual void cgfuncpostamble(Function func, const char *label) = 0;
    virtual Reg cgint2char(Reg reg) = 0;
    virtual Reg cgchar2int(Reg reg) = 0;
    virtual Reg cgfloat2int(Reg reg) = 0;
    virtual Reg cgint2float(Reg reg) = 0;
    virtual Reg cgfloat2char(Reg reg) = 0;
    virtual Reg cgchar2float(Reg reg) = 0;
    virtual Reg cgint2long(Reg reg) = 0;
    virtual Reg cglong2int(Reg reg) = 0;
    virtual Reg cgfloat2long(Reg reg) = 0;
    virtual Reg cglong2float(Reg reg) = 0;
    virtual Reg cglong2char(Reg reg) = 0;
    virtual Reg cgchar2long(Reg reg) = 0;
    virtual Reg cgcall(const char *name, const Reg reg) = 0;
    virtual void cgreturn(Reg reg, const char *end_label) = 0;
    virtual Reg cgaddress(Symbol identifier) = 0;
    virtual Reg cgderef(Reg reg, PrimitiveType type) = 0;
    virtual Reg cgshlconst(Reg reg, int value) = 0;
    virtual Reg cgstorderef(Reg reg, Reg addr, PrimitiveType type) = 0;
    virtual void cginc(Symbol identifier, PrimitiveType type) = 0;
    virtual void cgdec(Symbol identifier, PrimitiveType type) = 0;
    virtual void cginc(Reg addr, PrimitiveType type) = 0;
    virtual void cgdec(Reg addr, PrimitiveType type) = 0;
    virtual Reg cginvert(Reg reg) = 0;
    virtual Reg cgshl(Reg r1, Reg r2) = 0;
    virtual Reg cgshr(Reg r1, Reg r2) = 0;
    virtual Reg cgxor(Reg r1, Reg r2) = 0;
    virtual Reg cgand(Reg r1, Reg r2) = 0;
    virtual Reg cgor(Reg r1, Reg r2) = 0;
    virtual Reg cgparamaddr(Symbol identifier) = 0;
    virtual void cgresetparamcount() = 0;
};

