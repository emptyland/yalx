// Copyright 2018 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "arm64/asm-arm64.h"
#include "base/bit-ops.h"

namespace yalx {

namespace arm64 {

namespace {

inline uint64_t LargestPowerOf2Divisor(uint64_t value) {
    // Simulate two's complement (instead of casting to signed and negating) to
    // avoid undefined behavior on signed overflow.
    return value & ((~value) + 1);
}

inline int CountLeadingZeros(uint64_t value, int width) {
    assert(IsPowerOf2(width) && (width <= 64));
    if (value == 0) {
        return width;
    }
    return base::Bits::CountLeadingZeros64(value << (64 - width));
}

unsigned CalcLSPairDataSize(LoadStorePairOp op) {
    static_assert(kXRegSize == kDRegSize, "X and D registers must be same size.");
    static_assert(kWRegSize == kSRegSize, "W and S registers must be same size.");
    switch (op) {
        case STP_q:
        case LDP_q:
            return kQRegSizeLog2;
        case STP_x:
        case LDP_x:
        case STP_d:
        case LDP_d:
            return kXRegSizeLog2;
        default:
            return kWRegSizeLog2;
    }
}

} // internal

VectorFormat VectorFormatHalfWidth(VectorFormat vform) {
    assert(vform == kFormat8H || vform == kFormat4S || vform == kFormat2D ||
           vform == kFormatH || vform == kFormatS || vform == kFormatD);
    switch (vform) {
        case kFormat8H:
            return kFormat8B;
        case kFormat4S:
            return kFormat4H;
        case kFormat2D:
            return kFormat2S;
        case kFormatH:
            return kFormatB;
        case kFormatS:
            return kFormatH;
        case kFormatD:
            return kFormatS;
        default:
            UNREACHABLE();
    }
}

VectorFormat VectorFormatDoubleWidth(VectorFormat vform) {
  assert(vform == kFormat8B || vform == kFormat4H || vform == kFormat2S ||
         vform == kFormatB || vform == kFormatH || vform == kFormatS);
    switch (vform) {
        case kFormat8B:
            return kFormat8H;
        case kFormat4H:
            return kFormat4S;
        case kFormat2S:
            return kFormat2D;
        case kFormatB:
            return kFormatH;
        case kFormatH:
            return kFormatS;
        case kFormatS:
            return kFormatD;
        default:
            UNREACHABLE();
    }
}

VectorFormat VectorFormatFillQ(VectorFormat vform) {
    switch (vform) {
        case kFormatB:
        case kFormat8B:
        case kFormat16B:
            return kFormat16B;
        case kFormatH:
        case kFormat4H:
        case kFormat8H:
            return kFormat8H;
        case kFormatS:
        case kFormat2S:
        case kFormat4S:
            return kFormat4S;
        case kFormatD:
        case kFormat1D:
        case kFormat2D:
            return kFormat2D;
        default:
            UNREACHABLE();
    }
}

VectorFormat VectorFormatHalfWidthDoubleLanes(VectorFormat vform) {
    switch (vform) {
        case kFormat4H:
            return kFormat8B;
        case kFormat8H:
            return kFormat16B;
        case kFormat2S:
            return kFormat4H;
        case kFormat4S:
            return kFormat8H;
        case kFormat1D:
            return kFormat2S;
        case kFormat2D:
            return kFormat4S;
        default:
            UNREACHABLE();
    }
}

VectorFormat VectorFormatDoubleLanes(VectorFormat vform) {
    assert(vform == kFormat8B || vform == kFormat4H || vform == kFormat2S);
    switch (vform) {
        case kFormat8B:
            return kFormat16B;
        case kFormat4H:
            return kFormat8H;
        case kFormat2S:
            return kFormat4S;
        default:
            UNREACHABLE();
    }
}

VectorFormat VectorFormatHalfLanes(VectorFormat vform) {
    assert(vform == kFormat16B || vform == kFormat8H || vform == kFormat4S);
    switch (vform) {
        case kFormat16B:
            return kFormat8B;
        case kFormat8H:
            return kFormat4H;
        case kFormat4S:
            return kFormat2S;
        default:
            UNREACHABLE();
    }
}

VectorFormat ScalarFormatFromLaneSize(int laneSize) {
    switch (laneSize) {
        case 8:
            return kFormatB;
        case 16:
            return kFormatH;
        case 32:
            return kFormatS;
        case 64:
            return kFormatD;
        default:
            UNREACHABLE();
    }
}

VectorFormat VectorFormatFillQ(int laneSize) {
  return VectorFormatFillQ(ScalarFormatFromLaneSize(laneSize));
}

VectorFormat ScalarFormatFromFormat(VectorFormat vform) {
  return ScalarFormatFromLaneSize(LaneSizeInBitsFromFormat(vform));
}

unsigned RegisterSizeInBytesFromFormat(VectorFormat vform) {
  return RegisterSizeInBitsFromFormat(vform) / 8;
}

unsigned RegisterSizeInBitsFromFormat(VectorFormat vform) {
    assert(vform != kFormatUndefined);
    switch (vform) {
        case kFormatB:
            return kBRegSizeInBits;
        case kFormatH:
            return kHRegSizeInBits;
        case kFormatS:
            return kSRegSizeInBits;
        case kFormatD:
            return kDRegSizeInBits;
        case kFormat8B:
        case kFormat4H:
        case kFormat2S:
        case kFormat1D:
            return kDRegSizeInBits;
        default:
            return kQRegSizeInBits;
    }
}

unsigned LaneSizeInBitsFromFormat(VectorFormat vform) {
    assert(vform != kFormatUndefined);
    switch (vform) {
        case kFormatB:
        case kFormat8B:
        case kFormat16B:
            return 8;
        case kFormatH:
        case kFormat4H:
        case kFormat8H:
            return 16;
        case kFormatS:
        case kFormat2S:
        case kFormat4S:
            return 32;
        case kFormatD:
        case kFormat1D:
        case kFormat2D:
            return 64;
        default:
            UNREACHABLE();
    }
}

int LaneSizeInBytesFromFormat(VectorFormat vform) {
    return LaneSizeInBitsFromFormat(vform) / 8;
}

int LaneSizeInBytesLog2FromFormat(VectorFormat vform) {
    assert(vform != kFormatUndefined);
    switch (vform) {
        case kFormatB:
        case kFormat8B:
        case kFormat16B:
            return 0;
        case kFormatH:
        case kFormat4H:
        case kFormat8H:
            return 1;
        case kFormatS:
        case kFormat2S:
        case kFormat4S:
            return 2;
        case kFormatD:
        case kFormat1D:
        case kFormat2D:
            return 3;
        default:
            UNREACHABLE();
    }
}

int LaneCountFromFormat(VectorFormat vform) {
    assert(vform != kFormatUndefined);
    switch (vform) {
        case kFormat16B:
            return 16;
        case kFormat8B:
        case kFormat8H:
            return 8;
        case kFormat4H:
        case kFormat4S:
            return 4;
        case kFormat2S:
        case kFormat2D:
            return 2;
        case kFormat1D:
        case kFormatB:
        case kFormatH:
        case kFormatS:
        case kFormatD:
            return 1;
        default:
            UNREACHABLE();
    }
}

int MaxLaneCountFromFormat(VectorFormat vform) {
    assert(vform != kFormatUndefined);
    switch (vform) {
        case kFormatB:
        case kFormat8B:
        case kFormat16B:
            return 16;
        case kFormatH:
        case kFormat4H:
        case kFormat8H:
            return 8;
        case kFormatS:
        case kFormat2S:
        case kFormat4S:
            return 4;
        case kFormatD:
        case kFormat1D:
        case kFormat2D:
            return 2;
        default:
            UNREACHABLE();
    }
}

// Does 'vform' indicate a vector format or a scalar format?
bool IsVectorFormat(VectorFormat vform) {
    assert(vform != kFormatUndefined);
    switch (vform) {
        case kFormatB:
        case kFormatH:
        case kFormatS:
        case kFormatD:
            return false;
        default:
            return true;
    }
}

int64_t MaxIntFromFormat(VectorFormat vform) {
    return INT64_MAX >> (64 - LaneSizeInBitsFromFormat(vform));
}

int64_t MinIntFromFormat(VectorFormat vform) {
    return INT64_MIN >> (64 - LaneSizeInBitsFromFormat(vform));
}

uint64_t MaxUintFromFormat(VectorFormat vform) {
    return UINT64_MAX >> (64 - LaneSizeInBitsFromFormat(vform));
}


bool AreConsecutive(const VRegister& reg1, const VRegister& reg2, const VRegister& reg3, const VRegister& reg4) {
    assert(reg1.is_valid());
    if (!reg2.is_valid()) {
        assert(!reg3.is_valid() && !reg4.is_valid());
        return true;
    } else if (reg2.code() != ((reg1.code() + 1) % kNumberOfVRegisters)) {
        return false;
    }

    if (!reg3.is_valid()) {
        assert(!reg4.is_valid());
        return true;
    } else if (reg3.code() != ((reg2.code() + 1) % kNumberOfVRegisters)) {
        return false;
    }

    if (!reg4.is_valid()) {
        return true;
    } else if (reg4.code() != ((reg3.code() + 1) % kNumberOfVRegisters)) {
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
// MemOperand

MemOperand::MemOperand(Register base, const Operand& offset, AddrMode addrmode)
    : base_(base)
    , regoffset_(NoReg)
    , addrmode_(addrmode) {

    assert(base.Is64Bits() && !base.IsZero());

    if (offset.IsImmediate()) {
        offset_ = offset.immediate_value();
    } else if (offset.IsShiftedRegister()) {
        assert((addrmode == Offset) || (addrmode == PostIndex));

        regoffset_ = offset.reg();
        shift_ = offset.shift();
        shift_amount_ = offset.shift_amount();

        extend_ = NO_EXTEND;
        offset_ = 0;

        // These assertions match those in the shifted-register constructor.
        assert(regoffset_.Is64Bits() && !regoffset_.IsSP());
        assert(shift_ == LSL);
    } else {
        assert(offset.IsExtendedRegister());
        assert(addrmode == Offset);

        regoffset_ = offset.reg();
        extend_ = offset.extend();
        shift_amount_ = offset.shift_amount();

        shift_ = NO_SHIFT;
        offset_ = 0;

        // These assertions match those in the extended-register constructor.
        assert(!regoffset_.IsSP());
        assert((extend_ == UXTW) || (extend_ == SXTW) || (extend_ == SXTX));
        assert((regoffset_.Is64Bits() || (extend_ != SXTX)));
    }
}


//----------------------------------------------------------------------------------------------------------------------
// Assembler
void Assembler::movi(const VRegister& vd, const uint64_t imm, Shift shift, const int shift_amount) {
    
    assert((shift == LSL) || (shift == MSL));
    if (vd.Is2D() || vd.Is1D()) {
        assert(shift_amount == 0);
        int imm8 = 0;
        for (int i = 0; i < 8; ++i) {
            int byte = (imm >> (i * 8)) & 0xFF;
            assert((byte == 0) || (byte == 0xFF));
            if (byte == 0xFF) {
                imm8 |= (1 << i);
            }
        }
        uint32_t q = vd.Is2D() ? NEON_Q : 0;
        Emit(q | NEONModImmOp(1) | NEONModifiedImmediate_MOVI | ImmNEONabcdefgh(imm8) | NEONCmode(0xE) | Rd(vd));
    } else if (shift == LSL) {
        assert(base::is_uint8(imm));
        NEONModifiedImmShiftLsl(vd, static_cast<int>(imm), shift_amount, NEONModifiedImmediate_MOVI);
    } else {
        assert(base::is_uint8(imm));
        NEONModifiedImmShiftMsl(vd, static_cast<int>(imm), shift_amount, NEONModifiedImmediate_MOVI);
    }
}

void Assembler::ins(const VRegister& vd, int vd_index, const Register& rn) {
    // We support vd arguments of the form vd.VxT() or vd.T(), where x is the
    // number of lanes, and T is b, h, s or d.
    int lane_size = vd.LaneSizeInBytes();
    NEONFormatField format;
    switch (lane_size) {
      case 1:
          format = NEON_16B;
          assert(rn.IsW());
          break;
      case 2:
          format = NEON_8H;
            assert(rn.IsW());
          break;
      case 4:
          format = NEON_4S;
          assert(rn.IsW());
          break;
      default:
          assert(lane_size != 8);
          assert(rn.IsX());
          format = NEON_2D;
          break;
    }

    assert((0 <= vd_index) && (vd_index < LaneCountFromFormat(static_cast<VectorFormat>(format))));
    Emit(NEON_INS_GENERAL | ImmNEON5(format, vd_index) | Rn(rn) | Rd(vd));
}

void Assembler::smov(const Register& rd, const VRegister& vn, int vn_index) {
    // We support vn arguments of the form vn.VxT() or vn.T(), where x is the
    // number of lanes, and T is b, h, s.
    int lane_size = vn.LaneSizeInBytes();
    NEONFormatField format;
    uint32_t q = 0;
    switch (lane_size) {
        case 1:
            format = NEON_16B;
            break;
        case 2:
            format = NEON_8H;
            break;
        default:
            assert(lane_size == 4);
            assert(rd.IsX());
            format = NEON_4S;
            break;
    }
    q = rd.IsW() ? 0 : NEON_Q;
    assert((0 <= vn_index) && (vn_index < LaneCountFromFormat(static_cast<VectorFormat>(format))));
    Emit(q | NEON_SMOV | ImmNEON5(format, vn_index) | Rn(vn) | Rd(rd));
}

void Assembler::umov(const Register& rd, const VRegister& vn, int vn_index) {
    // We support vn arguments of the form vn.VxT() or vn.T(), where x is the
    // number of lanes, and T is b, h, s or d.
    int lane_size = vn.LaneSizeInBytes();
    NEONFormatField format;
    uint32_t q = 0;
    switch (lane_size) {
        case 1:
            format = NEON_16B;
            assert(rd.IsW());
            break;
        case 2:
            format = NEON_8H;
            assert(rd.IsW());
            break;
        case 4:
            format = NEON_4S;
            assert(rd.IsW());
            break;
        default:
            assert(lane_size == 8);
            assert(rd.IsX());
            format = NEON_2D;
            q = NEON_Q;
            break;
    }

    assert((0 <= vn_index) && (vn_index < LaneCountFromFormat(static_cast<VectorFormat>(format))));
    Emit(q | NEON_UMOV | ImmNEON5(format, vn_index) | Rn(vn) | Rd(rd));
}

void Assembler::dup(const VRegister& vd, const VRegister& vn, int vn_index) {
    // We support vn arguments of the form vn.VxT() or vn.T(), where x is the
    // number of lanes, and T is b, h, s or d.
    int lane_size = vn.LaneSizeInBytes();
    NEONFormatField format;
    switch (lane_size) {
        case 1:
            format = NEON_16B;
            break;
        case 2:
            format = NEON_8H;
            break;
        case 4:
            format = NEON_4S;
            break;
        default:
            assert(lane_size == 8);
            format = NEON_2D;
            break;
    }

    uint32_t q, scalar;
    if (vd.IsScalar()) {
        q = NEON_Q;
        scalar = NEONScalar;
    } else {
        assert(!vd.Is1D());
        q = vd.IsD() ? 0 : NEON_Q;
        scalar = 0;
    }
    Emit(q | scalar | NEON_DUP_ELEMENT | ImmNEON5(format, vn_index) | Rn(vn) | Rd(vd));
}

void Assembler::NEONModifiedImmShiftLsl(const VRegister& vd, const int imm8, const int left_shift,
                                        NEONModifiedImmediateOp op) {
    assert(vd.Is8B() || vd.Is16B() || vd.Is4H() || vd.Is8H() || vd.Is2S() || vd.Is4S());
    assert((left_shift == 0) || (left_shift == 8) || (left_shift == 16) || (left_shift == 24));
    assert(base::is_uint8(imm8));

    int cmode_1, cmode_2, cmode_3;
    if (vd.Is8B() || vd.Is16B()) {
        assert(op == NEONModifiedImmediate_MOVI);
        cmode_1 = 1;
        cmode_2 = 1;
        cmode_3 = 1;
    } else {
        cmode_1 = (left_shift >> 3) & 1;
        cmode_2 = left_shift >> 4;
        cmode_3 = 0;
        if (vd.Is4H() || vd.Is8H()) {
            assert((left_shift == 0) || (left_shift == 8));
            cmode_3 = 1;
        }
    }
    int cmode = (cmode_3 << 3) | (cmode_2 << 2) | (cmode_1 << 1);

    uint32_t q = vd.IsQ() ? NEON_Q : 0;
    Emit(q | op | ImmNEONabcdefgh(imm8) | NEONCmode(cmode) | Rd(vd));
}

void Assembler::Logical(const Register& rd, const Register& rn, const Operand& operand, LogicalOp op) {
    assert(rd.size_in_bits() == rn.size_in_bits());
    //assert(!operand.NeedsRelocation(this));
    if (operand.IsImmediate()) {
        int64_t immediate = operand.immediate_value();
        unsigned reg_size = rd.size_in_bits();

        assert(immediate != 0);
        assert(immediate != -1);
        assert(rd.Is64Bits() || base::is_uint32(immediate));

        // If the operation is NOT, invert the operation and immediate.
        if ((op & NOT) == NOT) {
        op = static_cast<LogicalOp>(op & ~NOT);
            immediate = rd.Is64Bits() ? ~immediate : (~immediate & kWRegMask);
        }

        unsigned n, imm_s, imm_r;
        if (IsImmLogical(immediate, reg_size, &n, &imm_s, &imm_r)) {
            // Immediate can be encoded in the instruction.
            LogicalImmediate(rd, rn, n, imm_s, imm_r, op);
        } else {
            // This case is handled in the macro assembler.
            UNREACHABLE();
        }
    } else {
        assert(operand.IsShiftedRegister());
        assert(operand.reg().size_in_bits() == rd.size_in_bits());
        uint32_t dp_op = static_cast<uint32_t>(op | LogicalShiftedFixed);
        DataProcShiftedRegister(rd, rn, operand, LeaveFlags, dp_op);
    }
}

// Code generation helpers.
void Assembler::MoveWide(const Register& rd, uint64_t imm, int shift, MoveWideImmediateOp mov_op) {
    // Ignore the top 32 bits of an immediate if we're moving to a W register.
    if (rd.Is32Bits()) {
        // Check that the top 32 bits are zero (a positive 32-bit number) or top
        // 33 bits are one (a negative 32-bit number, sign extended to 64 bits).
        assert(((imm >> kWRegSizeInBits) == 0) || ((imm >> (kWRegSizeInBits - 1)) == 0x1FFFFFFFF));
        imm &= kWRegMask;
    }

    if (shift >= 0) {
        // Explicit shift specified.
        assert((shift == 0) || (shift == 16) || (shift == 32) || (shift == 48));
        assert(rd.Is64Bits() || (shift == 0) || (shift == 16));
        shift /= 16;
    } else {
        // Calculate a new immediate and shift combination to encode the immediate
        // argument.
        shift = 0;
        if ((imm & ~0xFFFFULL) == 0) {
            // Nothing to do.
        } else if ((imm & ~(0xFFFFULL << 16)) == 0) {
            imm >>= 16;
            shift = 1;
        } else if ((imm & ~(0xFFFFULL << 32)) == 0) {
            assert(rd.Is64Bits());
            imm >>= 32;
            shift = 2;
        } else if ((imm & ~(0xFFFFULL << 48)) == 0) {
            assert(rd.Is64Bits());
            imm >>= 48;
            shift = 3;
        }
    }

    assert(base::is_uint16(imm));
    Emit(SF(rd) | MoveWideImmediateFixed | mov_op | Rd(rd) | ImmMoveWide(static_cast<int>(imm)) | ShiftMoveWide(shift));
}

void Assembler::AddSub(const Register& rd, const Register& rn, const Operand& operand, FlagsUpdate S, AddSubOp op) {
    assert(rd.size_in_bits() == rn.size_in_bits());
    //assert(!operand.NeedsRelocation(this));
    if (operand.IsImmediate()) {
        int64_t immediate = operand.immediate_value();
        assert(IsImmAddSub(immediate));
        uint32_t dest_reg = (S == SetFlags) ? Rd(rd) : RdSP(rd);
        Emit(SF(rd) | AddSubImmediateFixed | op | Flags(S) | ImmAddSub(static_cast<int>(immediate)) | dest_reg |
             RnSP(rn));
    } else if (operand.IsShiftedRegister()) {
        assert(operand.reg().size_in_bits() == rd.size_in_bits());
        assert(operand.shift() != ROR);

        // For instructions of the form:
        //   add/sub   wsp, <Wn>, <Wm> [, LSL #0-3 ]
        //   add/sub   <Wd>, wsp, <Wm> [, LSL #0-3 ]
        //   add/sub   wsp, wsp, <Wm> [, LSL #0-3 ]
        //   adds/subs <Wd>, wsp, <Wm> [, LSL #0-3 ]
        // or their 64-bit register equivalents, convert the operand from shifted to
        // extended register mode, and emit an add/sub extended instruction.
        if (rn.IsSP() || rd.IsSP()) {
            assert(!(rd.IsSP() && (S == SetFlags)));
            DataProcExtendedRegister(rd, rn, operand.ToExtendedRegister(), S, AddSubExtendedFixed | op);
        } else {
            DataProcShiftedRegister(rd, rn, operand, S, AddSubShiftedFixed | op);
        }
    } else {
        assert(operand.IsExtendedRegister());
        DataProcExtendedRegister(rd, rn, operand, S, AddSubExtendedFixed | op);
    }
}

// Test if a given value can be encoded in the immediate field of a logical
// instruction.
// If it can be encoded, the function returns true, and values pointed to by n,
// imm_s and imm_r are updated with immediates encoded in the format required
// by the corresponding fields in the logical instruction.
// If it can not be encoded, the function returns false, and the values pointed
// to by n, imm_s and imm_r are undefined.
bool Assembler::IsImmLogical(uint64_t value, unsigned width, unsigned* n, unsigned* imm_s, unsigned* imm_r) {
    assert((n != nullptr) && (imm_s != nullptr) && (imm_r != nullptr));
    assert((width == kWRegSizeInBits) || (width == kXRegSizeInBits));

    bool negate = false;

    // Logical immediates are encoded using parameters n, imm_s and imm_r using
    // the following table:
    //
    //    N   imms    immr    size        S             R
    //    1  ssssss  rrrrrr    64    UInt(ssssss)  UInt(rrrrrr)
    //    0  0sssss  xrrrrr    32    UInt(sssss)   UInt(rrrrr)
    //    0  10ssss  xxrrrr    16    UInt(ssss)    UInt(rrrr)
    //    0  110sss  xxxrrr     8    UInt(sss)     UInt(rrr)
    //    0  1110ss  xxxxrr     4    UInt(ss)      UInt(rr)
    //    0  11110s  xxxxxr     2    UInt(s)       UInt(r)
    // (s bits must not be all set)
    //
    // A pattern is constructed of size bits, where the least significant S+1 bits
    // are set. The pattern is rotated right by R, and repeated across a 32 or
    // 64-bit value, depending on destination register width.
    //
    // Put another way: the basic format of a logical immediate is a single
    // contiguous stretch of 1 bits, repeated across the whole word at intervals
    // given by a power of 2. To identify them quickly, we first locate the
    // lowest stretch of 1 bits, then the next 1 bit above that; that combination
    // is different for every logical immediate, so it gives us all the
    // information we need to identify the only logical immediate that our input
    // could be, and then we simply check if that's the value we actually have.
    //
    // (The rotation parameter does give the possibility of the stretch of 1 bits
    // going 'round the end' of the word. To deal with that, we observe that in
    // any situation where that happens the bitwise NOT of the value is also a
    // valid logical immediate. So we simply invert the input whenever its low bit
    // is set, and then we know that the rotated case can't arise.)

    if (value & 1) {
        // If the low bit is 1, negate the value, and set a flag to remember that we
        // did (so that we can adjust the return values appropriately).
        negate = true;
        value = ~value;
    }

    if (width == kWRegSizeInBits) {
        // To handle 32-bit logical immediates, the very easiest thing is to repeat
        // the input value twice to make a 64-bit word. The correct encoding of that
        // as a logical immediate will also be the correct encoding of the 32-bit
        // value.

        // The most-significant 32 bits may not be zero (ie. negate is true) so
        // shift the value left before duplicating it.
        value <<= kWRegSizeInBits;
        value |= value >> kWRegSizeInBits;
    }

    // The basic analysis idea: imagine our input word looks like this.
    //
    //    0011111000111110001111100011111000111110001111100011111000111110
    //                                                          c  b    a
    //                                                          |<--d-->|
    //
    // We find the lowest set bit (as an actual power-of-2 value, not its index)
    // and call it a. Then we add a to our original number, which wipes out the
    // bottommost stretch of set bits and replaces it with a 1 carried into the
    // next zero bit. Then we look for the new lowest set bit, which is in
    // position b, and subtract it, so now our number is just like the original
    // but with the lowest stretch of set bits completely gone. Now we find the
    // lowest set bit again, which is position c in the diagram above. Then we'll
    // measure the distance d between bit positions a and c (using CLZ), and that
    // tells us that the only valid logical immediate that could possibly be equal
    // to this number is the one in which a stretch of bits running from a to just
    // below b is replicated every d bits.
    uint64_t a = LargestPowerOf2Divisor(value);
    uint64_t value_plus_a = value + a;
    uint64_t b = LargestPowerOf2Divisor(value_plus_a);
    uint64_t value_plus_a_minus_b = value_plus_a - b;
    uint64_t c = LargestPowerOf2Divisor(value_plus_a_minus_b);

    int d, clz_a, out_n;
    uint64_t mask;

    if (c != 0) {
        // The general case, in which there is more than one stretch of set bits.
        // Compute the repeat distance d, and set up a bitmask covering the basic
        // unit of repetition (i.e. a word with the bottom d bits set). Also, in all
        // of these cases the N bit of the output will be zero.
        clz_a = CountLeadingZeros(a, kXRegSizeInBits);
        int clz_c = CountLeadingZeros(c, kXRegSizeInBits);
        d = clz_a - clz_c;
        mask = ((uint64_t{1} << d) - 1);
        out_n = 0;
    } else {
        // Handle degenerate cases.
        //
        // If any of those 'find lowest set bit' operations didn't find a set bit at
        // all, then the word will have been zero thereafter, so in particular the
        // last lowest_set_bit operation will have returned zero. So we can test for
        // all the special case conditions in one go by seeing if c is zero.
        if (a == 0) {
            // The input was zero (or all 1 bits, which will come to here too after we
            // inverted it at the start of the function), for which we just return
            // false.
            return false;
        } else {
          // Otherwise, if c was zero but a was not, then there's just one stretch
          // of set bits in our word, meaning that we have the trivial case of
          // d == 64 and only one 'repetition'. Set up all the same variables as in
          // the general case above, and set the N bit in the output.
          clz_a = CountLeadingZeros(a, kXRegSizeInBits);
          d = 64;
          mask = ~uint64_t{0};
          out_n = 1;
        }
    }
    
    // If the repeat period d is not a power of two, it can't be encoded.
    if (!IsPowerOf2(d)) {
        return false;
    }

    if (((b - a) & ~mask) != 0) {
        // If the bit stretch (b - a) does not fit within the mask derived from the
        // repeat period, then fail.
        return false;
    }

    // The only possible option is b - a repeated every d bits. Now we're going to
    // actually construct the valid logical immediate derived from that
    // specification, and see if it equals our original input.
    //
    // To repeat a value every d bits, we multiply it by a number of the form
    // (1 + 2^d + 2^(2d) + ...), i.e. 0x0001000100010001 or similar. These can
    // be derived using a table lookup on CLZ(d).
    static const uint64_t multipliers[] = {
        0x0000000000000001UL, 0x0000000100000001UL, 0x0001000100010001UL,
        0x0101010101010101UL, 0x1111111111111111UL, 0x5555555555555555UL,
    };
    int multiplier_idx = CountLeadingZeros(d, kXRegSizeInBits) - 57;
    // Ensure that the index to the multipliers array is within bounds.
    assert((multiplier_idx >= 0) && (static_cast<size_t>(multiplier_idx) < arraysize(multipliers)));
    uint64_t multiplier = multipliers[multiplier_idx];
    uint64_t candidate = (b - a) * multiplier;

    if (value != candidate) {
        // The candidate pattern doesn't match our input value, so fail.
        return false;
    }

    // We have a match! This is a valid logical immediate, so now we have to
    // construct the bits and pieces of the instruction encoding that generates
    // it.

    // Count the set bits in our basic stretch. The special case of clz(0) == -1
    // makes the answer come out right for stretches that reach the very top of
    // the word (e.g. numbers like 0xFFFFC00000000000).
    int clz_b = (b == 0) ? -1 : CountLeadingZeros(b, kXRegSizeInBits);
    int s = clz_a - clz_b;

    // Decide how many bits to rotate right by, to put the low bit of that basic
    // stretch in position a.
    int r;
    if (negate) {
        // If we inverted the input right at the start of this function, here's
        // where we compensate: the number of set bits becomes the number of clear
        // bits, and the rotation count is based on position b rather than position
        // a (since b is the location of the 'lowest' 1 bit after inversion).
        s = d - s;
        r = (clz_b + 1) & (d - 1);
    } else {
        r = (clz_a + 1) & (d - 1);
    }

    // Now we're done, except for having to encode the S output in such a way that
    // it gives both the number of set bits and the length of the repeated
    // segment. The s field is encoded like this:
    //
    //     imms    size        S
    //    ssssss    64    UInt(ssssss)
    //    0sssss    32    UInt(sssss)
    //    10ssss    16    UInt(ssss)
    //    110sss     8    UInt(sss)
    //    1110ss     4    UInt(ss)
    //    11110s     2    UInt(s)
    //
    // So we 'or' (-d * 2) with our computed s to form imms.
    *n = out_n;
    *imm_s = ((-d * 2) | (s - 1)) & 0x3F;
    *imm_r = r;

    return true;
}

void Assembler::LoadStorePair(const CPURegister& rt, const CPURegister& rt2,
                              const MemOperand& addr, LoadStorePairOp op) {
    // 'rt' and 'rt2' can only be aliased for stores.
    assert(((op & LoadStorePairLBit) == 0) || rt != rt2);
    assert(AreSameSizeAndType(rt, rt2));
    assert(IsImmLSPair(addr.offset(), CalcLSPairDataSize(op)));
    int offset = static_cast<int>(addr.offset());

    uint32_t memop = op | Rt(rt) | Rt2(rt2) | RnSP(addr.base()) | ImmLSPair(offset, CalcLSPairDataSize(op));

    uint32_t addrmodeop;
    if (addr.IsImmediateOffset()) {
        addrmodeop = LoadStorePairOffsetFixed;
    } else {
        // Pre-index and post-index modes.
        assert(rt != addr.base());
        assert(rt2 != addr.base());
        assert(addr.offset() != 0);
        if (addr.IsPreIndex()) {
            addrmodeop = LoadStorePairPreIndexFixed;
        } else {
            assert(addr.IsPostIndex());
            addrmodeop = LoadStorePairPostIndexFixed;
        }
    }
    Emit(addrmodeop | memop);
}

void Assembler::LoadStore(const CPURegister& rt, const MemOperand& addr, LoadStoreOp op) {
    uint32_t memop = op | Rt(rt) | RnSP(addr.base());

    if (addr.IsImmediateOffset()) {
        unsigned size = CalcLSDataSize(op);
        if (IsImmLSScaled(addr.offset(), size)) {
            int offset = static_cast<int>(addr.offset());
            // Use the scaled addressing mode.
            Emit(LoadStoreUnsignedOffsetFixed | memop | ImmLSUnsigned(offset >> size));
        } else if (IsImmLSUnscaled(addr.offset())) {
            int offset = static_cast<int>(addr.offset());
            // Use the unscaled addressing mode.
            Emit(LoadStoreUnscaledOffsetFixed | memop | ImmLS(offset));
        } else {
            // This case is handled in the macro assembler.
            UNREACHABLE();
        }
    } else if (addr.IsRegisterOffset()) {
        Extend ext = addr.extend();
        Shift shift = addr.shift();
        unsigned shift_amount = addr.shift_amount();

        // LSL is encoded in the option field as UXTX.
        if (shift == LSL) {
            ext = UXTX;
        }

        // Shifts are encoded in one bit, indicating a left shift by the memory
        // access size.
        assert((shift_amount == 0) || (shift_amount == static_cast<unsigned>(CalcLSDataSize(op))));
        Emit(LoadStoreRegisterOffsetFixed | memop | Rm(addr.regoffset()) | ExtendMode(ext) |
             ImmShiftLS((shift_amount > 0) ? 1 : 0));
    } else {
        // Pre-index and post-index modes.
        assert(rt != addr.base());
        if (IsImmLSUnscaled(addr.offset())) {
            int offset = static_cast<int>(addr.offset());
            if (addr.IsPreIndex()) {
                Emit(LoadStorePreIndexFixed | memop | ImmLS(offset));
            } else {
                assert(addr.IsPostIndex());
                Emit(LoadStorePostIndexFixed | memop | ImmLS(offset));
            }
        } else {
            // This case is handled in the macro assembler.
            UNREACHABLE();
        }
    }
}

} // namespace arm64

} // namespace yalx
