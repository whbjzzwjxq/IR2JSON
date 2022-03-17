#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/raw_ostream.h"

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
std::string out;
raw_string_ostream buffer(out);

namespace {
struct IR2JSON : public FunctionPass {
    static char ID;
    IR2JSON() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
        json::Object func_info;
        func_info["name"] = F.getName().str();
        func_info["basic_blocks"] = {};
        for (auto &bb : F.getBasicBlockList()) {
            json::Object bb_info;
            bb_info["name"] = bb.getName().str();
            bb_info["insts"] = {};
            for (auto &inst : bb.getInstList()) {
                json::Object inst_info;
                inst_info["name"] = inst.getName().str();
                inst_info["opcode"] = inst.getOpcodeName();
                inst_info["operands"] = {};
                for (auto &i : inst.operands()) {
                    json::Object op_info;
                    auto op = i.get();
                    op_info["name"] = op->getName().str();
                    op_info["type"] = enumToStr(static_cast<int>(op->getType()->getTypeID()));
                    op->printAsOperand(buffer, false);
                    op_info["value"] = out;
                    out = "";
                    auto _val = json::Value(json::Object(op_info));
                    inst_info["operands"].getAsArray()->push_back(_val);
                };
                auto _val = json::Value(json::Object(inst_info));
                bb_info["insts"].getAsArray()->push_back(_val);
            };
            auto _val = json::Value(json::Object(bb_info));
            func_info["basic_blocks"].getAsArray()->push_back(_val);
        };
        json::OStream J(errs());
        J.value(json::Value(json::Object(func_info)));
        errs() << "\n";
        return false;
    };
};
} // namespace

char IR2JSON::ID = 0;
static RegisterPass<IR2JSON> X("ir2json", "Covert IR to JSON", false, false);
