#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

std::string _prefix = "    ";
static const char *TypeIDStrings[] = {
    "Half",      ///< 16-bit floating point type
    "BFloat",    ///< 16-bit floating point type (7-bit significand)
    "Float",     ///< 32-bit floating point type
    "Double",    ///< 64-bit floating point type
    "X86_FP80",  ///< 80-bit floating point type (X87)
    "FP128",     ///< 128-bit floating point type (112-bit significand)
    "PPC_FP128", ///< 128-bit floating point type (two 64-bits, PowerPC)
    "Void",      ///< type with no size
    "Label",     ///< Labels
    "Metadata",  ///< Metadata
    "X86_MMX",   ///< MMX vectors (64 bits, X86 specific)
    "X86_AMX",   ///< AMX vectors (8192 bits, X86 specific)
    "Token",     ///< Tokens

    // Derived types... see DerivedTypes.h file.
    "Integer",        ///< Arbitrary bit width integers
    "Function",       ///< Functions
    "Pointer",        ///< Pointers
    "Struct",         ///< Structures
    "Array",          ///< Arrays
    "FixedVector",    ///< Fixed width SIMD vector type
    "ScalableVector", ///< Scalable SIMD vector type
};

const char *enumToStr(int enumVal) { return TypeIDStrings[enumVal]; }
std::string out_buffer;
raw_string_ostream buffer(out_buffer);

namespace {
struct IR2JSON : public FunctionPass {
    static char ID;
    IR2JSON() : FunctionPass(ID) {}

    json::Value obj2value(json::Object obj) {
        auto _val = json::Value(json::Object(obj));
        return _val;
    };

    json::Value opd2json(Value *opd) {
        json::Object obj;
        obj["var_name"] = opd->getName().str();
        obj["type"] = enumToStr(static_cast<int>(opd->getType()->getTypeID()));
        std::string out_buffer;
        raw_string_ostream buffer(out_buffer);
        opd->printAsOperand(buffer, false);
        obj["value"] = out_buffer;
        out_buffer.clear();
        return obj2value(obj);
    };

    json::Value inst2json(Instruction &inst) {
        json::Object inst_info;
        inst_info["assign"] = inst.getName().str();
        inst_info["opcode"] = inst.getOpcodeName();
        inst_info["operands"] = {};
        if (llvm::CmpInst *cmpInst = dyn_cast<llvm::CmpInst>(&inst)) {
            inst_info["predicate"] = cmpInst->getPredicateName(cmpInst->getPredicate());
        } else {
            inst_info["predicate"] = "";
        }
        for (auto &i : inst.operands()) {
            inst_info["operands"].getAsArray()->push_back(opd2json(i.get()));
        };
        return obj2value(inst_info);
    };

    json::Value bb2json(BasicBlock &bb) {
        json::Object bb_info;
        bb_info["name"] = bb.getName().str();
        bb_info["insts"] = {};
        for (auto &inst : bb.getInstList()) {
            bb_info["insts"].getAsArray()->push_back(inst2json(inst));
        };
        return obj2value(bb_info);
    }

    json::Object func2json(Function &F) {
        json::Object func_info;
        func_info["name"] = F.getName().str();
        func_info["ret_type"] = enumToStr(F.getReturnType()->getTypeID());
        func_info["basic_blocks"] = {};
        func_info["args"] = {};
        for (auto &arg : F.args()) {
            func_info["args"].getAsArray()->push_back(opd2json(&arg));
        }
        for (auto &bb : F.getBasicBlockList()) {
            func_info["basic_blocks"].getAsArray()->push_back(bb2json(bb));
        };
        return func_info;
    }

    bool runOnFunction(Function &F) override {
        json::Object func_info = func2json(F);
        json::OStream J(errs());
        J.value(json::Value(json::Object(func_info)));
        errs() << "\n";
        return false;
    };
};
} // namespace

char IR2JSON::ID = 0;
static RegisterPass<IR2JSON> X("ir2json", "Covert IR to JSON", false, false);
