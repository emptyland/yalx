#include "arm64/instr-arm64.h"
#include "arm64/asm-arm64.h"


namespace yalx {

namespace arm64 {

void Instruction::SetImmPCOffsetTarget(const Instruction* target) {
    if (IsPCRelAddressing()) {
        SetPCRelImmTarget(target);
    } else if (BranchType() != UnknownBranchType) {
        SetBranchImmTarget(target);
    } /*else if (IsUnresolvedInternalReference()) {
        SetUnresolvedInternalReferenceImmTarget(options, target);
    }*/ else {
        // Load literal (offset from PC).
        SetImmLLiteral(target);
    }
}

void Instruction::SetPCRelImmTarget(const Instruction* target) {
    // ADRP is not supported, so 'this' must point to an ADR instruction.
    assert(IsAdr());

    ptrdiff_t target_offset = DistanceTo(target);
    if (Instruction::IsValidPCRelOffset(target_offset)) {
        uint32_t imm = Assembler::ImmPCRelAddress(static_cast<int>(target_offset));
        SetBits(Mask(~ImmPCRel_mask) | imm);
    } else {
        UNREACHABLE();
    }
}

void Instruction::SetBranchImmTarget(const Instruction* target) {
    assert(IsAligned(DistanceTo(target), kInstrSize));
    assert(IsValidImmPCOffset(BranchType(), DistanceTo(target)));
    int offset = static_cast<int>(DistanceTo(target) >> kInstrSizeLog2);
    uint32_t branch_imm = 0;
    uint32_t imm_mask = 0;
    switch (BranchType()) {
        case CondBranchType: {
            branch_imm = Assembler::ImmCondBranch(offset);
            imm_mask = ImmCondBranch_mask;
            break;
        }
        case UncondBranchType: {
            branch_imm = Assembler::ImmUncondBranch(offset);
            imm_mask = ImmUncondBranch_mask;
            break;
        }
        case CompareBranchType: {
            branch_imm = Assembler::ImmCmpBranch(offset);
            imm_mask = ImmCmpBranch_mask;
            break;
        }
        case TestBranchType: {
            branch_imm = Assembler::ImmTestBranch(offset);
            imm_mask = ImmTestBranch_mask;
            break;
        }
        default:
            UNREACHABLE();
    }
    SetBits(Mask(~imm_mask) | branch_imm);
}

void Instruction::SetImmLLiteral(const Instruction* source) {
    assert(IsLdrLiteral());
    assert(IsAligned(DistanceTo(source), kInstrSize));
    assert(Assembler::IsImmLLiteral(DistanceTo(source)));
    uint32_t imm = Assembler::ImmLLiteral(static_cast<int>(DistanceTo(source) >> kLoadLiteralScaleLog2));
    uint32_t mask = ImmLLiteral_mask;

    SetBits(Mask(~mask) | imm);
}

int64_t Instruction::ImmPCOffset() {
    int64_t offset;
    if (IsPCRelAddressing()) {
        // PC-relative addressing. Only ADR is supported.
        offset = ImmPCRel();
    } else if (BranchType() != UnknownBranchType) {
        // All PC-relative branches.
        // Relative branch offsets are instruction-size-aligned.
        offset = ImmBranch() * kInstrSize;
    } /*else if (IsUnresolvedInternalReference()) {
        // Internal references are always word-aligned.
        offset = ImmUnresolvedInternalReference() * kInstrSize;
    }*/ else {
        // Load literal (offset from PC).
        assert(IsLdrLiteral());
        // The offset is always shifted by 2 bits, even for loads to 64-bits
        // registers.
        offset = ImmLLiteral() * kInstrSize;
    }
    return offset;
}

int Instruction::ImmBranch() const {
    switch (BranchType()) {
        case CondBranchType:
            return ImmCondBranch();
        case UncondBranchType:
            return ImmUncondBranch();
        case CompareBranchType:
            return ImmCmpBranch();
        case TestBranchType:
            return ImmTestBranch();
        default:
            UNREACHABLE();
    }
    return 0;
}

ImmBranchType Instruction::BranchType() const {
    if (IsCondBranchImm()) {
        return CondBranchType;
    } else if (IsUncondBranchImm()) {
        return UncondBranchType;
    } else if (IsCompareBranch()) {
        return CompareBranchType;
    } else if (IsTestBranch()) {
        return TestBranchType;
    } else {
        return UnknownBranchType;
    }
}

int Instruction::ImmBranchRangeBitwidth(ImmBranchType branch_type) {
    switch (branch_type) {
        case UncondBranchType:
            return ImmUncondBranch_width;
        case CondBranchType:
            return ImmCondBranch_width;
        case CompareBranchType:
            return ImmCmpBranch_width;
        case TestBranchType:
            return ImmTestBranch_width;
        default:
            UNREACHABLE();
    }
}

} // namespace arm64


} // namespace yalx
