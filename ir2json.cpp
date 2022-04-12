#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

static std::string TypeIDStrings[] = {
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

namespace {
struct IR2JSON : public FunctionPass {
    static char ID;
    IR2JSON() : FunctionPass(ID) {}

    std::string type2str(llvm::Type *_type) {
        if (llvm::IntegerType *intType = dyn_cast<llvm::IntegerType>(_type)) {
            std::string s_type = "i";
            s_type.append(std::to_string(intType->getBitWidth()));
            return s_type;
        }
        return TypeIDStrings[static_cast<int>(_type->getTypeID())];
    }

    json::Value obj2value(json::Object obj) {
        auto _val = json::Value(json::Object(obj));
        return _val;
    };

    std::string printValue(Value *opd) {
        std::string out_buffer;
        raw_string_ostream buffer(out_buffer);
        opd->printAsOperand(buffer, false);
        return out_buffer;
    }

    json::Object val2json(Value *opd) {
        json::Object obj;
        obj["var_name"] = opd->getName().str();
        obj["type"] = type2str(opd->getType());
        obj["value"] = printValue(opd);
        return obj;
    };

    json::Object use2json(Use *opd, Instruction &inst) {
        json::Object obj = val2json(opd->get());
        if (llvm::isa<PHINode>(inst)) {
            if (llvm::PHINode *phiInst = dyn_cast<llvm::PHINode>(&inst)) {
                llvm::BasicBlock *bb = phiInst->getIncomingBlock(*opd);
                obj["pred_block"] = bb->getName().str();
            };
        }
        return obj;
    };

    json::Object alloca2json(AllocaInst &inst) {
        json::Object alloca_info;
        alloca_info["align"] = inst.getAlignment();
        alloca_info["allocated_type"] = type2str(inst.getAllocatedType());
        alloca_info["array_size"] = printValue(inst.getArraySize());
        alloca_info["isArray"] = inst.isArrayAllocation();
        return alloca_info;
    }

    json::Object inst2json(Instruction &inst) {
        json::Object inst_info;
        inst_info["assign"] = inst.getName().str();
        inst_info["opcode"] = inst.getOpcodeName();
        inst_info["operands"] = {};
        if (llvm::CmpInst *cmpInst = dyn_cast<llvm::CmpInst>(&inst)) {
            inst_info["predicate"] = cmpInst->getPredicateName(cmpInst->getPredicate());
        }
        if (llvm::AllocaInst *allocaInst = dyn_cast<llvm::AllocaInst>(&inst)) {
            inst_info["alloca"] = obj2value(alloca2json(*allocaInst));
        }

        for (auto &i : inst.operands()) {
            auto opd_info = use2json(&i, inst);
            inst_info["operands"].getAsArray()->push_back(obj2value(opd_info));
        };
        return inst_info;
    };

    json::Object bb2json(BasicBlock &bb) {
        json::Object bb_info;
        bb_info["name"] = bb.getName().str();
        bb_info["insts"] = {};
        for (auto &inst : bb.getInstList()) {
            bb_info["insts"].getAsArray()->push_back(obj2value(inst2json(inst)));
        };
        return bb_info;
    }

    json::Object func2json(Function &F) {
        json::Object func_info;
        func_info["name"] = F.getName().str();
        func_info["ret_type"] = type2str(F.getReturnType());
        func_info["basic_blocks"] = {};
        func_info["args"] = {};
        for (auto &arg : F.args()) {
            func_info["args"].getAsArray()->push_back(
                obj2value(val2json(&arg)));
        }
        for (auto &bb : F.getBasicBlockList()) {
            func_info["basic_blocks"].getAsArray()->push_back(obj2value(bb2json(bb)));
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
