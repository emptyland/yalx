#include "ir/pass/constants-folding.h"

namespace yalx {

namespace ir {

ConstantsFoldingPass::ConstantsFoldingPass(base::Arena *arena, ModulesMap *modules, cpl::SyntaxFeedback *feedback)
: Pass<ConstantsFoldingPass>(arena, modules, feedback) {
}

void ConstantsFoldingPass::RunModule(Module *module) {
    for (size_t i = 0; i < module->values_size(); i++) {
        if (module->value(i)->IsAlive()) {
            auto value = module->value(i);
            (*module->mutable_values())[i] = FoldGlobalValue(value);
        }
    }
    
    ForeachUdt(module);
    ForeachFunction(module);
}

void ConstantsFoldingPass::RunFun(Function *fun) {
    
    ForeachBasicBlock(fun);
    
    for (auto blk : fun->blocks()) {
        blk->RemoveDeads();
    }
}

void ConstantsFoldingPass::RunBasicBlock(BasicBlock *block) {
    
}

Value *ConstantsFoldingPass::FoldGlobalValue(Value *value) {
    int store_count = 0;
    Value *store = nullptr;
    for (auto edge : value->users()) {
        if (edge.user->Is(Operator::kStoreGlobal)) {
            store = edge.user;
            store_count++;
        }
    }

    if (store_count == 1) {
        bool folded = false;
        Value *new_val = nullptr;
        if (store->InputValue(1)->op()->IsConstant()) {
            new_val = store->InputValue(1);
            folded = true;
        } else {
            new_val = FoldValueIfNeeded(store->InputValue(1), &folded);
        }
        if (folded) {
            std::vector<Value *> loads;
            for (auto edge : value->users()) {
                if (edge.user->Is(Operator::kStoreGlobal)) {
                    edge.user->Kill();
                } else if (edge.user->Is(Operator::kLoadGlobal)) {
                    loads.push_back(edge.user);
                }
            }
            
            for (auto load : loads) {
                for (auto [position, user] : load->GetUsers()) {
                    user->Replace(arena(), position, load, new_val);
                }
                load->Kill();
            }
            return new_val;
        }
    }

    return value;
}

template<class T>
struct AddOperator {
    inline Value *Apply(T lhs, T rhs) { return nullptr; }
};

Value *ConstantsFoldingPass::FoldValueIfNeeded(Value *input, bool *folded) {
    for (auto i = 0; i < input->op()->value_in(); i++) {
        if (!input->InputValue(i)->op()->IsConstant()) {
            *folded = false;
            return input;
        }
    }
    
    switch (input->op()->value()) {
        case Operator::kAdd: {
            switch (input->InputValue(0)->op()->value()) {
                case Operator::kWord8Constant:
                case Operator::kU8Constant:
                    
                    break;
                    
                default:
                    break;
            }
        } break;
        case Operator::kFAdd: {
        } break;
        case Operator::kSub:
            break;
        case Operator::kMul:
            break;
        case Operator::kFMul:
            break;
        case Operator::kFDiv:
            break;
        case Operator::kSDiv:
            break;
        case Operator::kUDiv:
            break;
            
        default:
            *folded = false;
            break;
    }
    return input;
}

} // namespace ir

} // namespace yalx
