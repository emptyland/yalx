// Copyright 2013 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef YALX_ARM64_INSTR_ARM64_H_
#define YALX_ARM64_INSTR_ARM64_H_

#include "arm64/const-arm64.h"
#include "base/utils.h"
#include "base/checking.h"
#include "base/base.h"
#include <string.h>

namespace yalx {

namespace arm64 {

enum ImmBranchType {
    UnknownBranchType = 0,
    CondBranchType = 1,
    UncondBranchType = 2,
    CompareBranchType = 3,
    TestBranchType = 4
};

enum AddrMode { Offset, PreIndex, PostIndex };

enum FPRounding {
    // The first four values are encodable directly by FPCR<RMode>.
    FPTieEven = 0x0,
    FPPositiveInfinity = 0x1,
    FPNegativeInfinity = 0x2,
    FPZero = 0x3,

    // The final rounding modes are only available when explicitly specified by
    // the instruction (such as with fcvta). They cannot be set in FPCR.
    FPTieAway,
    FPRoundOdd
};

enum Reg31Mode { Reg31IsStackPointer, Reg31IsZeroRegister };

// Instructions. ---------------------------------------------------------------

class Instruction final {
public:

#define DEFINE_GETTER(Name, HighBit, LowBit, Func) \
    int32_t Name() const { return Func(HighBit, LowBit); }
    INSTRUCTION_FIELDS_LIST(DEFINE_GETTER)
#undef DEFINE_GETTER
    
    void SetImmPCOffsetTarget(const Instruction* target);
    
    void SetPCRelImmTarget(const Instruction* target);
    void SetBranchImmTarget(const Instruction* target);
    void SetImmLLiteral(const Instruction* source);
    
    int64_t ImmPCOffset();
    
    // ImmPCRel is a compound field (not present in INSTRUCTION_FIELDS_LIST),
    // formed from ImmPCRelLo and ImmPCRelHi.
    int ImmPCRel() const {
        assert(IsPCRelAddressing());
        int offset = (static_cast<uint32_t>(ImmPCRelHi()) << ImmPCRelLo_width) | ImmPCRelLo();
        int width = ImmPCRelLo_width + ImmPCRelHi_width;
        return base::signed_bitextract_32(width - 1, 0, offset);
    }
    
    int ImmBranch() const;
    
    static uint32_t ImmLLiteral(int imm19) {
        assert(base::is_int19(imm19));
        return base::truncate_to_int19(imm19) << ImmLLiteral_offset;
    }
    
    ImmBranchType BranchType() const;
    
    static int ImmBranchRangeBitwidth(ImmBranchType branch_type);
    
    bool IsValidImmPCOffset(ImmBranchType branch_type, ptrdiff_t offset) {
        assert(offset % kInstrSize == 0);
        return base::is_intn(offset / kInstrSize, ImmBranchRangeBitwidth(branch_type));
    }
    
    // Helpers.
    bool IsCondBranchImm() const { return Mask(ConditionalBranchFMask) == ConditionalBranchFixed; }

    bool IsUncondBranchImm() const { return Mask(UnconditionalBranchFMask) == UnconditionalBranchFixed; }

    bool IsCompareBranch() const { return Mask(CompareBranchFMask) == CompareBranchFixed; }

    bool IsTestBranch() const { return Mask(TestBranchFMask) == TestBranchFixed; }

    bool IsImmBranch() const { return BranchType() != UnknownBranchType; }

    bool IsLdrLiteral() const { return Mask(LoadLiteralFMask) == LoadLiteralFixed; }

    bool IsLdrLiteralX() const { return Mask(LoadLiteralMask) == LDR_x_lit; }
    bool IsLdrLiteralW() const { return Mask(LoadLiteralMask) == LDR_w_lit; }

    bool IsPCRelAddressing() const { return Mask(PCRelAddressingFMask) == PCRelAddressingFixed; }

    bool IsAdr() const { return Mask(PCRelAddressingMask) == ADR; }

    bool IsBrk() const { return Mask(ExceptionMask) == BRK; }

//    bool IsUnresolvedInternalReference() const {
//      // Unresolved internal references are encoded as two consecutive brk
//      // instructions.
//      return IsBrk() && following()->IsBrk();
//    }

    bool IsLogicalImmediate() const { return Mask(LogicalImmediateFMask) == LogicalImmediateFixed; }

    bool IsAddSubImmediate() const { return Mask(AddSubImmediateFMask) == AddSubImmediateFixed; }

    bool IsAddSubShifted() const { return Mask(AddSubShiftedFMask) == AddSubShiftedFixed; }

    bool IsAddSubExtended() const { return Mask(AddSubExtendedFMask) == AddSubExtendedFixed; }

    // Match any loads or stores, including pairs.
    bool IsLoadOrStore() const { return Mask(LoadStoreAnyFMask) == LoadStoreAnyFixed; }
    
    ptrdiff_t DistanceTo(const Instruction* target) {
        return reinterpret_cast<const uint8_t *>(target) - reinterpret_cast<Address>(this);
    }
    
    uint32_t bits() const { return *reinterpret_cast<const uint32_t *>(this); }
    
    void SetBits(uint32_t instr) { ::memcpy(this, &instr, sizeof(instr)); }
    
    uint32_t Bits(int msb, int lsb) const { return base::unsigned_bitextract_32(msb, lsb, bits()); }

    int32_t SignedBits(int msb, int lsb) const {
        // Usually this is aligned, but when de/serializing that's not guaranteed.
        int32_t vals = *reinterpret_cast<const int32_t *>(this);
        return base::signed_bitextract_32(msb, lsb, vals);
    }
    
    uint32_t Mask(uint32_t mask) const { return bits() & mask; }
    
    static bool IsValidPCRelOffset(ptrdiff_t offset) { return base::is_int21(offset); }

    DISALLOW_IMPLICIT_CONSTRUCTORS(Instruction);
private:
    
}; // class Instruction


} // namespace arm64

} // namespace yalx

#endif // YALX_ARM64_INSTR_ARM64_H_
