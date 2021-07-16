#include "ir/scope.h"


namespace yalx {

namespace ir {

IRCodeBranchScope::IRCodeBranchScope(IRCodeEnvScope **location, cpl::Statement *ast, BasicBlock *block,
                                     IRCodeBranchScope *trunk)
    : IRCodeEnvScope(location)
    , ast_(ast)
    , block_(block)
    , branch_id_(0) {
    if (trunk) {
        trunk->branchs_.push_back(this);
        branch_id_ = static_cast<int>(trunk->branchs_.size());
    } else {
        Enter();
    }
}
IRCodeBranchScope::~IRCodeBranchScope() {
    if (IsTrunk()) {
        Exit();
    }
    for (auto br : branchs_) {
        delete br;
    }
}

void IRCodeBranchScope::Update(std::string_view name, IRCodeEnvScope *owns, Value *value) {
    // TODO:
    UNREACHABLE();
    if (auto br = owns->NearlyBranchScope(); br == owns) {
        br->values_[name] = value;
    }
}

} // namespace ir

} // namespace yalx
