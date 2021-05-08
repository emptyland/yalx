// Copyright 2018 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "arm64/asm-arm64.h"

namespace yalx {

namespace arm64 {

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


//----------------------------------------------------------------------------------------------------------------------
// Assembler

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

} // namespace arm64

} // namespace yalx
