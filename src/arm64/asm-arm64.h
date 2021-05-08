// Copyright 2018 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#ifndef YALX_ARM64_ASM_ARM64_H_
#define YALX_ARM64_ASM_ARM64_H_

#include "arm64/const-arm64.h"
#include "base/utils.h"
#include "base/base.h"
#include "base/checking.h"
#include <string>

namespace yalx {

namespace arm64 {

// -----------------------------------------------------------------------------
// Registers.
// clang-format off
#define GENERAL_REGISTER_CODE_LIST(R)                     \
  R(0)  R(1)  R(2)  R(3)  R(4)  R(5)  R(6)  R(7)          \
  R(8)  R(9)  R(10) R(11) R(12) R(13) R(14) R(15)         \
  R(16) R(17) R(18) R(19) R(20) R(21) R(22) R(23)         \
  R(24) R(25) R(26) R(27) R(28) R(29) R(30) R(31)

#define GENERAL_REGISTERS(R)                              \
  R(x0)  R(x1)  R(x2)  R(x3)  R(x4)  R(x5)  R(x6)  R(x7)  \
  R(x8)  R(x9)  R(x10) R(x11) R(x12) R(x13) R(x14) R(x15) \
  R(x16) R(x17) R(x18) R(x19) R(x20) R(x21) R(x22) R(x23) \
  R(x24) R(x25) R(x26) R(x27) R(x28) R(x29) R(x30) R(x31)

// x18 is the platform register and is reserved for the use of platform ABIs.
// It is known to be reserved by the OS at least on Windows and iOS.
#define ALWAYS_ALLOCATABLE_GENERAL_REGISTERS(R)                  \
  R(x0)  R(x1)  R(x2)  R(x3)  R(x4)  R(x5)  R(x6)  R(x7)  \
  R(x8)  R(x9)  R(x10) R(x11) R(x12) R(x13) R(x14) R(x15) \
         R(x19) R(x20) R(x21) R(x22) R(x23) R(x24) R(x25) \
  R(x27)

#define ALLOCATABLE_GENERAL_REGISTERS(V)  \
  ALWAYS_ALLOCATABLE_GENERAL_REGISTERS(V) \
  MAYBE_ALLOCATABLE_GENERAL_REGISTERS(V)

#define FLOAT_REGISTERS(V)                                \
  V(s0)  V(s1)  V(s2)  V(s3)  V(s4)  V(s5)  V(s6)  V(s7)  \
  V(s8)  V(s9)  V(s10) V(s11) V(s12) V(s13) V(s14) V(s15) \
  V(s16) V(s17) V(s18) V(s19) V(s20) V(s21) V(s22) V(s23) \
  V(s24) V(s25) V(s26) V(s27) V(s28) V(s29) V(s30) V(s31)

#define DOUBLE_REGISTERS(R)                               \
  R(d0)  R(d1)  R(d2)  R(d3)  R(d4)  R(d5)  R(d6)  R(d7)  \
  R(d8)  R(d9)  R(d10) R(d11) R(d12) R(d13) R(d14) R(d15) \
  R(d16) R(d17) R(d18) R(d19) R(d20) R(d21) R(d22) R(d23) \
  R(d24) R(d25) R(d26) R(d27) R(d28) R(d29) R(d30) R(d31)

#define SIMD128_REGISTERS(V)                              \
  V(q0)  V(q1)  V(q2)  V(q3)  V(q4)  V(q5)  V(q6)  V(q7)  \
  V(q8)  V(q9)  V(q10) V(q11) V(q12) V(q13) V(q14) V(q15) \
  V(q16) V(q17) V(q18) V(q19) V(q20) V(q21) V(q22) V(q23) \
  V(q24) V(q25) V(q26) V(q27) V(q28) V(q29) V(q30) V(q31)

#define VECTOR_REGISTERS(V)                               \
  V(v0)  V(v1)  V(v2)  V(v3)  V(v4)  V(v5)  V(v6)  V(v7)  \
  V(v8)  V(v9)  V(v10) V(v11) V(v12) V(v13) V(v14) V(v15) \
  V(v16) V(v17) V(v18) V(v19) V(v20) V(v21) V(v22) V(v23) \
  V(v24) V(v25) V(v26) V(v27) V(v28) V(v29) V(v30) V(v31)

// Register d29 could be allocated, but we keep an even length list here, in
// order to make stack alignment easier for save and restore.
#define ALLOCATABLE_DOUBLE_REGISTERS(R)                   \
  R(d0)  R(d1)  R(d2)  R(d3)  R(d4)  R(d5)  R(d6)  R(d7)  \
  R(d8)  R(d9)  R(d10) R(d11) R(d12) R(d13) R(d14) R(d16) \
  R(d17) R(d18) R(d19) R(d20) R(d21) R(d22) R(d23) R(d24) \
  R(d25) R(d26) R(d27) R(d28)
// clang-format on

// Some CPURegister methods can return Register and VRegister types, so we
// need to declare them in advance.
class Register;
class VRegister;

enum RegisterCode {
#define REGISTER_CODE(R) kRegCode_##R,
    GENERAL_REGISTERS(REGISTER_CODE)
#undef REGISTER_CODE
    kRegAfterLast
}; // enum RegisterCode


class CPURegister {
public:
    enum RegisterType { kRegister, kVRegister, kNoRegister };
    
    static constexpr int kCode_no_reg = -1;
    
    static constexpr CPURegister NoReg() { return CPURegister{kCode_no_reg, 0, kNoRegister}; }
    
    static constexpr CPURegister Create(int code, int size, RegisterType type) {
        assert(IsValid(code, size, type));
        return CPURegister{code, size, type};
    }
    
    bool is_valid() const { return code() != kCode_no_reg; }
    
    int size_in_bits() const { return reg_size_; }
    
    int size_in_bytes() const { return size_in_bits() / 8; }
    
    bool Is8Bits() const { return size_in_bits() == 8; }
    
    bool Is16Bits() const { return size_in_bits() == 16; }
    
    bool Is32Bits() const { return size_in_bits() == 32; }
    
    bool Is64Bits() const { return size_in_bits() == 64; }
    
    bool Is128Bits() const { return size_in_bits() == 128; }
    
    bool IsNone() const { return type() == kNoRegister; }
    
    constexpr bool operator == (const CPURegister &other) const {
        return code_ == other.code() && reg_size_ == other.size_in_bits() && type_ == other.type();
    }
    
    constexpr bool operator != (const CPURegister &other) const { return !operator ==(other); }
    
    bool IsZero() const { return IsRegister() && (code() == kZeroRegCode); }
    bool IsSP() const { return IsRegister() && (code() == kSPRegInternalCode); }

    bool IsRegister() const { return type() == kRegister; }
    bool IsVRegister() const { return type() == kVRegister; }

    bool IsFPRegister() const { return IsS() || IsD(); }

    bool IsW() const { return IsRegister() && Is32Bits(); }
    bool IsX() const { return IsRegister() && Is64Bits(); }

    // These assertions ensure that the size and type of the register are as
    // described. They do not consider the number of lanes that make up a vector.
    // So, for example, Is8B() implies IsD(), and Is1D() implies IsD, but IsD()
    // does not imply Is1D() or Is8B().
    // Check the number of lanes, ie. the format of the vector, using methods such
    // as Is8B(), Is1D(), etc. in the VRegister class.
    bool IsV() const { return IsVRegister(); }
    bool IsB() const { return IsV() && Is8Bits(); }
    bool IsH() const { return IsV() && Is16Bits(); }
    bool IsS() const { return IsV() && Is32Bits(); }
    bool IsD() const { return IsV() && Is64Bits(); }
    bool IsQ() const { return IsV() && Is128Bits(); }
    
    inline Register Reg() const;
    inline VRegister VReg() const;

    inline Register X() const;
    inline Register W() const;
    inline VRegister V() const;
    inline VRegister B() const;
    inline VRegister H() const;
    inline VRegister D() const;
    inline VRegister S() const;
    inline VRegister Q() const;

    DEF_VAL_GETTER(RegisterType, type);
    DEF_VAL_GETTER(int, code);
    
protected:
    constexpr CPURegister(int code, int reg_size, RegisterType type)
        : code_(code)
        , reg_size_(reg_size)
        , type_(type) {}
    
    static constexpr bool IsValidRegister(int code, int size) {
        return (size == kWRegSizeInBits || size == kXRegSizeInBits) &&
               (code < kNumberOfRegisters || code == kSPRegInternalCode);
    }

    static constexpr bool IsValidVRegister(int code, int size) {
        return (size == kBRegSizeInBits || size == kHRegSizeInBits ||
                size == kSRegSizeInBits || size == kDRegSizeInBits || size == kQRegSizeInBits) &&
                code < kNumberOfVRegisters;
    }

    static constexpr bool IsValid(int code, int size, RegisterType type) {
        return (type == kRegister && IsValidRegister(code, size)) ||
               (type == kVRegister && IsValidVRegister(code, size));
    }

    static constexpr bool IsNone(int code, int size, RegisterType type) {
        return type == kNoRegister && code == 0 && size == 0;
    }
    
    int code_;
    int reg_size_;
    RegisterType type_;
}; // class CPURegister



class Register final : public CPURegister {
public:
    static constexpr Register NoReg() { return Register(CPURegister::NoReg()); }

    static constexpr Register Create(int code, int size) {
        return Register(CPURegister::Create(code, size, CPURegister::kRegister));
    }

    static Register XRegFromCode(unsigned code);
    static Register WRegFromCode(unsigned code);

    static constexpr Register from_code(int code) {
        // Always return an X register.
        return Register::Create(code, kXRegSizeInBits);
    }

    static const char* GetSpecialRegisterName(int code) {
        return (code == kSPRegInternalCode) ? "sp" : "UNKNOWN";
    }
    
private:
    constexpr explicit Register(const CPURegister& r) : CPURegister(r) {}
}; // class Register


// Stack frame alignment and padding.
constexpr int ArgumentPaddingSlots(int argument_count) {
    // Stack frames are aligned to 16 bytes.
    constexpr int kStackFrameAlignment = 16;
    constexpr int alignment_mask = kStackFrameAlignment / kPointerSize - 1;
    return argument_count & alignment_mask;
}

constexpr bool kSimpleFPAliasing = true;
constexpr bool kSimdMaskRegisters = false;

enum DoubleRegisterCode {
#define REGISTER_CODE(R) kDoubleCode_##R,
    DOUBLE_REGISTERS(REGISTER_CODE)
#undef REGISTER_CODE
    kDoubleAfterLast
};

// Functions for handling NEON vector format information.
enum VectorFormat {
    kFormatUndefined = 0xffffffff,
    kFormat8B = NEON_8B,
    kFormat16B = NEON_16B,
    kFormat4H = NEON_4H,
    kFormat8H = NEON_8H,
    kFormat2S = NEON_2S,
    kFormat4S = NEON_4S,
    kFormat1D = NEON_1D,
    kFormat2D = NEON_2D,

    // Scalar formats. We add the scalar bit to distinguish between scalar and
    // vector enumerations; the bit is always set in the encoding of scalar ops
    // and always clear for vector ops. Although kFormatD and kFormat1D appear
    // to be the same, their meaning is subtly different. The first is a scalar
    // operation, the second a vector operation that only affects one lane.
    kFormatB = NEON_B | NEONScalar,
    kFormatH = NEON_H | NEONScalar,
    kFormatS = NEON_S | NEONScalar,
    kFormatD = NEON_D | NEONScalar
};


VectorFormat VectorFormatHalfWidth(VectorFormat vform);
VectorFormat VectorFormatDoubleWidth(VectorFormat vform);
VectorFormat VectorFormatDoubleLanes(VectorFormat vform);
VectorFormat VectorFormatHalfLanes(VectorFormat vform);
VectorFormat ScalarFormatFromLaneSize(int lanesize);
VectorFormat VectorFormatHalfWidthDoubleLanes(VectorFormat vform);
VectorFormat VectorFormatFillQ(int laneSize);
VectorFormat VectorFormatFillQ(VectorFormat vform);
VectorFormat ScalarFormatFromFormat(VectorFormat vform);
unsigned RegisterSizeInBitsFromFormat(VectorFormat vform);
unsigned RegisterSizeInBytesFromFormat(VectorFormat vform);
int LaneSizeInBytesFromFormat(VectorFormat vform);
unsigned LaneSizeInBitsFromFormat(VectorFormat vform);
int LaneSizeInBytesLog2FromFormat(VectorFormat vform);
int LaneCountFromFormat(VectorFormat vform);
int MaxLaneCountFromFormat(VectorFormat vform);
bool IsVectorFormat(VectorFormat vform);
int64_t MaxIntFromFormat(VectorFormat vform);
int64_t MinIntFromFormat(VectorFormat vform);
uint64_t MaxUintFromFormat(VectorFormat vform);


class VRegister final : public CPURegister {
public:
    static constexpr VRegister NoReg() { return VRegister(CPURegister::NoReg(), 0); }

    static constexpr VRegister Create(int code, int size, int lane_count = 1) {
        assert(IsValidLaneCount(lane_count));
        return VRegister(CPURegister::Create(code, size, CPURegister::kVRegister), lane_count);
    }
    
    static VRegister Create(int reg_code, VectorFormat format) {
        int reg_size = RegisterSizeInBitsFromFormat(format);
        int reg_count = IsVectorFormat(format) ? LaneCountFromFormat(format) : 1;
        return VRegister::Create(reg_code, reg_size, reg_count);
    }
    
    static VRegister BRegFromCode(unsigned code);
    static VRegister HRegFromCode(unsigned code);
    static VRegister SRegFromCode(unsigned code);
    static VRegister DRegFromCode(unsigned code);
    static VRegister QRegFromCode(unsigned code);
    static VRegister VRegFromCode(unsigned code);

    VRegister V8B() const { return VRegister::Create(code(), kDRegSizeInBits, 8); }
    VRegister V16B() const { return VRegister::Create(code(), kQRegSizeInBits, 16); }
    VRegister V4H() const { return VRegister::Create(code(), kDRegSizeInBits, 4); }
    VRegister V8H() const { return VRegister::Create(code(), kQRegSizeInBits, 8); }
    VRegister V2S() const { return VRegister::Create(code(), kDRegSizeInBits, 2); }
    VRegister V4S() const { return VRegister::Create(code(), kQRegSizeInBits, 4); }
    VRegister V2D() const { return VRegister::Create(code(), kQRegSizeInBits, 2); }
    VRegister V1D() const { return VRegister::Create(code(), kDRegSizeInBits, 1); }

    VRegister Format(VectorFormat f) const { return VRegister::Create(code(), f); }

    bool Is8B() const { return (Is64Bits() && (lane_count() == 8)); }
    bool Is16B() const { return (Is128Bits() && (lane_count() == 16)); }
    bool Is4H() const { return (Is64Bits() && (lane_count() == 4)); }
    bool Is8H() const { return (Is128Bits() && (lane_count() == 8)); }
    bool Is2S() const { return (Is64Bits() && (lane_count() == 2)); }
    bool Is4S() const { return (Is128Bits() && (lane_count() == 4)); }
    bool Is1D() const { return (Is64Bits() && (lane_count() == 1)); }
    bool Is2D() const { return (Is128Bits() && (lane_count() == 2)); }
    
    // For consistency, we assert the number of lanes of these scalar registers,
    // even though there are no vectors of equivalent total size with which they
    // could alias.
    bool Is1B() const {
        assert(!(Is8Bits() && IsVector()));
        return Is8Bits();
    }
    bool Is1H() const {
        assert(!(Is16Bits() && IsVector()));
        return Is16Bits();
    }
    bool Is1S() const {
        assert(!(Is32Bits() && IsVector()));
        return Is32Bits();
    }

    bool IsLaneSizeB() const { return LaneSizeInBits() == kBRegSizeInBits; }
    bool IsLaneSizeH() const { return LaneSizeInBits() == kHRegSizeInBits; }
    bool IsLaneSizeS() const { return LaneSizeInBits() == kSRegSizeInBits; }
    bool IsLaneSizeD() const { return LaneSizeInBits() == kDRegSizeInBits; }

    bool IsScalar() const { return lane_count_ == 1; }
    bool IsVector() const { return lane_count_ > 1; }

    bool IsSameFormat(const VRegister& other) const {
      return (reg_size_ == other.reg_size_) && (lane_count_ == other.lane_count_);
    }

    //int LaneCount() const { return lane_count_; }

    unsigned LaneSizeInBytes() const { return size_in_bytes() / lane_count(); }

    unsigned LaneSizeInBits() const { return LaneSizeInBytes() * 8; }

    static constexpr int kMaxNumRegisters = kNumberOfVRegisters;
    static_assert(kMaxNumRegisters == kDoubleAfterLast, "");

    static VRegister from_code(int code) {
      // Always return a D register.
      return VRegister::Create(code, kDRegSizeInBits);
    }
    
    DEF_VAL_GETTER(int, lane_count);
    
private:
    constexpr explicit VRegister(const CPURegister& r, int lane_count)
        : CPURegister(r), lane_count_(lane_count) {}
    
    
    static constexpr bool IsValidLaneCount(int lane_count) { return IsPowerOf2(lane_count) && lane_count <= 16; }
    
    int lane_count_;
}; // class VRegister


// No*Reg is used to indicate an unused argument, or an error case. Note that
// these all compare equal. The Register and VRegister variants are provided for
// convenience.
constexpr Register NoReg = Register::NoReg();
constexpr VRegister NoVReg = VRegister::NoReg();
constexpr CPURegister NoCPUReg = CPURegister::NoReg();
constexpr Register no_reg = NoReg;
constexpr VRegister no_dreg = NoVReg;

#define DEFINE_REGISTER(register_class, name, ...) \
    constexpr register_class name = register_class::Create(__VA_ARGS__)
#define ALIAS_REGISTER(register_class, alias, name) \
    constexpr register_class alias = name

#define DEFINE_REGISTERS(N)                            \
    DEFINE_REGISTER(Register, w##N, N, kWRegSizeInBits); \
    DEFINE_REGISTER(Register, x##N, N, kXRegSizeInBits);
GENERAL_REGISTER_CODE_LIST(DEFINE_REGISTERS)
#undef DEFINE_REGISTERS

DEFINE_REGISTER(Register, wsp, kSPRegInternalCode, kWRegSizeInBits);
DEFINE_REGISTER(Register, sp, kSPRegInternalCode, kXRegSizeInBits);

#define DEFINE_VREGISTERS(N)                            \
    DEFINE_REGISTER(VRegister, b##N, N, kBRegSizeInBits); \
    DEFINE_REGISTER(VRegister, h##N, N, kHRegSizeInBits); \
    DEFINE_REGISTER(VRegister, s##N, N, kSRegSizeInBits); \
    DEFINE_REGISTER(VRegister, d##N, N, kDRegSizeInBits); \
    DEFINE_REGISTER(VRegister, q##N, N, kQRegSizeInBits); \
    DEFINE_REGISTER(VRegister, v##N, N, kQRegSizeInBits);
GENERAL_REGISTER_CODE_LIST(DEFINE_VREGISTERS)
#undef DEFINE_VREGISTERS

#undef DEFINE_REGISTER

// Registers aliases.
//ALIAS_REGISTER(VRegister, v8_, v8);  // Avoid conflicts with namespace v8.
ALIAS_REGISTER(Register, ip0, x16);
ALIAS_REGISTER(Register, ip1, x17);
ALIAS_REGISTER(Register, wip0, w16);
ALIAS_REGISTER(Register, wip1, w17);
// Root register.
ALIAS_REGISTER(Register, kRootRegister, x26);
ALIAS_REGISTER(Register, rr, x26);
// Pointer cage base register.
#ifdef V8_COMPRESS_POINTERS_IN_SHARED_CAGE
ALIAS_REGISTER(Register, kPtrComprCageBaseRegister, x28);
#else
ALIAS_REGISTER(Register, kPtrComprCageBaseRegister, kRootRegister);
#endif
// Context pointer register.
ALIAS_REGISTER(Register, cp, x27);
ALIAS_REGISTER(Register, fp, x29);
ALIAS_REGISTER(Register, lr, x30);
ALIAS_REGISTER(Register, xzr, x31);
ALIAS_REGISTER(Register, wzr, w31);

// Register used for padding stack slots.
ALIAS_REGISTER(Register, padreg, x31);

// Keeps the 0 double value.
ALIAS_REGISTER(VRegister, fp_zero, d15);
// MacroAssembler fixed V Registers.
// d29 is not part of ALLOCATABLE_DOUBLE_REGISTERS, so use 27 and 28.
ALIAS_REGISTER(VRegister, fp_fixed1, d27);
ALIAS_REGISTER(VRegister, fp_fixed2, d28);

// MacroAssembler scratch V registers.
ALIAS_REGISTER(VRegister, fp_scratch, d30);
ALIAS_REGISTER(VRegister, fp_scratch1, d30);
ALIAS_REGISTER(VRegister, fp_scratch2, d31);

#undef ALIAS_REGISTER


inline Register Register::XRegFromCode(unsigned code) {
    if (code == kSPRegInternalCode) {
        return sp;
    } else {
        assert(code < static_cast<unsigned>(kNumberOfRegisters));
        return Register::Create(code, kXRegSizeInBits);
    }
}

inline Register Register::WRegFromCode(unsigned code) {
    if (code == kSPRegInternalCode) {
        return wsp;
    } else {
        assert(code < static_cast<unsigned>(kNumberOfRegisters));
        return Register::Create(code, kWRegSizeInBits);
    }
}

inline VRegister VRegister::BRegFromCode(unsigned code) {
    assert(code < static_cast<unsigned>(kNumberOfVRegisters));
    return VRegister::Create(code, kBRegSizeInBits);
}

inline VRegister VRegister::HRegFromCode(unsigned code) {
    assert(code < static_cast<unsigned>(kNumberOfVRegisters));
    return VRegister::Create(code, kHRegSizeInBits);
}

inline VRegister VRegister::SRegFromCode(unsigned code) {
    assert(code < static_cast<unsigned>(kNumberOfVRegisters));
    return VRegister::Create(code, kSRegSizeInBits);
}

inline VRegister VRegister::DRegFromCode(unsigned code) {
    assert(code < static_cast<unsigned>(kNumberOfVRegisters));
    return VRegister::Create(code, kDRegSizeInBits);
}

inline VRegister VRegister::QRegFromCode(unsigned code) {
    assert(code < static_cast<unsigned>(kNumberOfVRegisters));
    return VRegister::Create(code, kQRegSizeInBits);
}

inline VRegister VRegister::VRegFromCode(unsigned code) {
    assert(code < static_cast<unsigned>(kNumberOfVRegisters));
    return VRegister::Create(code, kVRegSizeInBits);
}

inline Register CPURegister::W() const {
    assert(IsRegister());
    return Register::WRegFromCode(code());
}

inline Register CPURegister::Reg() const {
    assert(IsRegister());
    return Register::Create(code(), reg_size_);
}

inline VRegister CPURegister::VReg() const {
    assert(IsVRegister());
    return VRegister::Create(code(), reg_size_);
}

inline Register CPURegister::X() const {
    assert(IsRegister());
    return Register::XRegFromCode(code());
}

inline VRegister CPURegister::V() const {
    assert(IsVRegister());
    return VRegister::VRegFromCode(code());
}

inline VRegister CPURegister::B() const {
    assert(IsVRegister());
    return VRegister::BRegFromCode(code());
}

inline VRegister CPURegister::H() const {
    assert(IsVRegister());
    return VRegister::HRegFromCode(code());
}

inline VRegister CPURegister::S() const {
    assert(IsVRegister());
    return VRegister::SRegFromCode(code());
}

inline VRegister CPURegister::D() const {
    assert(IsVRegister());
    return VRegister::DRegFromCode(code());
}

inline VRegister CPURegister::Q() const {
    assert(IsVRegister());
    return VRegister::QRegFromCode(code());
}


//----------------------------------------------------------------------------------------------------------------------

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

class Label;

//----------------------------------------------------------------------------------------------------------------------
// Immediate

//----------------------------------------------------------------------------------------------------------------------
// Operand

class Operand {
public:
    // rm, {<shift> {#<shift_amount>}}
    // where <shift> is one of {LSL, LSR, ASR, ROR}.
    //       <shift_amount> is uint6_t.
    // This is allowed to be an implicit constructor because Operand is
    // a wrapper class that doesn't normally perform any type conversion.
    Operand(Register reg, Shift shift = LSL, unsigned shift_amount = 0)
        : immediate_(0)
        , reg_(reg)
        , shift_(shift)
        , extend_(NO_EXTEND)
        , shift_amount_(shift_amount) {  // NOLINT(runtime/explicit)
        assert(reg.Is64Bits() || (shift_amount < kWRegSizeInBits));
        assert(reg.Is32Bits() || (shift_amount < kXRegSizeInBits));
        //DCHECK_IMPLIES(reg.IsSP(), shift_amount == 0);
    }

    // rm, <extend> {#<shift_amount>}
    // where <extend> is one of {UXTB, UXTH, UXTW, UXTX, SXTB, SXTH, SXTW, SXTX}.
    //       <shift_amount> is uint2_t.
    Operand(Register reg, Extend extend, unsigned shift_amount = 0)
        : immediate_(0)
        , reg_(reg)
        , shift_(NO_SHIFT)
        , extend_(extend)
        , shift_amount_(shift_amount) {
        assert(reg.is_valid());
        assert(shift_amount <= 4);
        assert(!reg.IsSP());

        // Extend modes SXTX and UXTX require a 64-bit register.
        assert(reg.Is64Bits() || ((extend != SXTX) && (extend != UXTX)));
    }
    
    Operand ToExtendedRegister() const {
        assert(IsShiftedRegister());
        assert((shift_ == LSL) && (shift_amount_ <= 4));
        return Operand(reg_, reg_.Is64Bits() ? UXTX : UXTW, shift_amount_);
    }

    Operand ToW() const {
        if (IsShiftedRegister()) {
            assert(reg_.Is64Bits());
            return Operand(reg_.W(), shift(), shift_amount());
        } else if (IsExtendedRegister()) {
            assert(reg_.Is64Bits());
            return Operand(reg_.W(), extend(), shift_amount());
        }
        assert(IsImmediate());
        return *this;
    }
    
    bool IsImmediate() const { return reg_ == NoReg; }

    bool IsShiftedRegister() const { return reg_.is_valid() && (shift_ != NO_SHIFT); }

    bool IsExtendedRegister() const { return reg_.is_valid() && (extend_ != NO_EXTEND); }

    bool IsZero() const {
        if (IsImmediate()) {
            return immediate_value() == 0;
        } else {
            return reg().IsZero();
        }
    }

    int64_t immediate_value() const { return immediate_; }
    DEF_VAL_GETTER(Register, reg);
    DEF_VAL_GETTER(Shift, shift);
    DEF_VAL_GETTER(Extend, extend);
    DEF_VAL_GETTER(unsigned, shift_amount);

private:
    int64_t immediate_;
    Register reg_;
    Shift shift_;
    Extend extend_;
    unsigned shift_amount_;
}; // class Operand


//----------------------------------------------------------------------------------------------------------------------
// Assembler


class Assembler {
public:
    inline Assembler() = default;
    
    
    
    // Instruction set functions ---------------------------------------------------------------------------------------

    // Branch / Jump instructions.
    // For branches offsets are scaled, i.e. in instructions not in bytes.
    // Branch to register.
    void br(const Register& xn) {
        assert(xn.Is64Bits());
        Emit(BR | Rn(xn));
    }

    // Branch-link to register.
    void blr(const Register& xn) {
        assert(xn.Is64Bits());
        // The pattern 'blr xzr' is used as a guard to detect when execution falls
        // through the constant pool. It should not be emitted.
        assert(xn != xzr);
        Emit(BLR | Rn(xn));
    }

    // Branch to register with return hint.
    void ret(const Register& xn = lr) {
        assert(xn.Is64Bits());
        Emit(RET | Rn(xn));
    }
    
    // Unconditional branch to label.
    void b(Label* label) { b(LinkAndGetInstructionOffsetTo(label)); }

    // Conditional branch to label.
    void b(Label* label, Condition cond) { b(LinkAndGetInstructionOffsetTo(label), cond); }

    // Unconditional branch to PC offset.
    void b(int imm26) { Emit(B | ImmUncondBranch(imm26)); }

    // Conditional branch to PC offset.
    void b(int imm19, Condition cond) { Emit(B_cond | ImmCondBranch(imm19) | cond); }

    // Branch-link to label / pc offset.
    void bl(Label* label) { bl(LinkAndGetInstructionOffsetTo(label)); }
    
    void bl(int imm26) { Emit(BL | ImmUncondBranch(imm26)); }
    
    // Compare and branch to label / pc offset if zero.
    void cbz(const Register& rt, Label* label) { cbz(rt, LinkAndGetInstructionOffsetTo(label)); }
    void cbz(const Register& rt, int imm19) { Emit(SF(rt) | CBZ | ImmCmpBranch(imm19) | Rt(rt)); }

    // Compare and branch to label / pc offset if not zero.
    void cbnz(const Register& rt, Label* label) { cbnz(rt, LinkAndGetInstructionOffsetTo(label)); }
    void cbnz(const Register& rt, int imm19) { Emit(SF(rt) | CBNZ | ImmCmpBranch(imm19) | Rt(rt)); }

    // Test bit and branch to label / pc offset if zero.
    void tbz(const Register& rt, unsigned bit_pos, Label* label) {
        tbz(rt, bit_pos, LinkAndGetInstructionOffsetTo(label));
    }
    void tbz(const Register& rt, unsigned bit_pos, int imm14) {
        assert(rt.Is64Bits() || (rt.Is32Bits() && (bit_pos < kWRegSizeInBits)));
        Emit(TBZ | ImmTestBranchBit(bit_pos) | ImmTestBranch(imm14) | Rt(rt));
    }

    // Test bit and branch to label / pc offset if not zero.
    void tbnz(const Register& rt, unsigned bit_pos, Label* label) {
        tbnz(rt, bit_pos, LinkAndGetInstructionOffsetTo(label));
    }
    void tbnz(const Register& rt, unsigned bit_pos, int imm14) {
        assert(rt.Is64Bits() || (rt.Is32Bits() && (bit_pos < kWRegSizeInBits)));
        Emit(TBNZ | ImmTestBranchBit(bit_pos) | ImmTestBranch(imm14) | Rt(rt));
    }

    // Address calculation instructions.
    // Calculate a PC-relative address. Unlike for branches the offset in adr is
    // unscaled (i.e. the result can be unaligned).
    void adr(const Register& rd, Label* label) { adr(rd, LinkAndGetByteOffsetTo(label)); }
    void adr(const Register& rd, int imm21) {
        assert(rd.Is64Bits());
        Emit(ADR | ImmPCRelAddress(imm21) | Rd(rd));
    }
    
    // Data Processing instructions.
    // Add.
    void add(const Register& rd, const Register& rn, const Operand& operand) {
        AddSub(rd, rn, operand, LeaveFlags, ADD);
    }

    // Add and update status flags.
    void adds(const Register& rd, const Register& rn, const Operand& operand) {
        AddSub(rd, rn, operand, SetFlags, ADD);
    }

    // Compare negative.
    void cmn(const Register& rn, const Operand& operand) {
        Register zr = AppropriateZeroRegFor(rn);
        adds(zr, rn, operand);
    }

    // Subtract.
    void sub(const Register& rd, const Register& rn, const Operand& operand) {
        AddSub(rd, rn, operand, LeaveFlags, SUB);
    }

    // Subtract and update status flags.
    void subs(const Register& rd, const Register& rn, const Operand& operand) {
        AddSub(rd, rn, operand, SetFlags, SUB);
    }

    // Compare.
    void cmp(const Register& rn, const Operand& operand) {
        Register zr = AppropriateZeroRegFor(rn);
        subs(zr, rn, operand);
    }

    // Negate.
    void neg(const Register& rd, const Operand& operand) {
        Register zr = AppropriateZeroRegFor(rd);
        sub(rd, zr, operand);
    }

    // Negate and update status flags.
    void negs(const Register& rd, const Operand& operand) {
        Register zr = AppropriateZeroRegFor(rd);
        subs(rd, zr, operand);
    }
    
    // Add with carry bit.
    void adc(const Register& rd, const Register& rn, const Operand& operand);

    // Add with carry bit and update status flags.
    void adcs(const Register& rd, const Register& rn, const Operand& operand);

    // Subtract with carry bit.
    void sbc(const Register& rd, const Register& rn, const Operand& operand);

    // Subtract with carry bit and update status flags.
    void sbcs(const Register& rd, const Register& rn, const Operand& operand);

    // Negate with carry bit.
    void ngc(const Register& rd, const Operand& operand);

    // Negate with carry bit and update status flags.
    void ngcs(const Register& rd, const Operand& operand);

    // Logical instructions.
    // Bitwise and (A & B).
    void and_(const Register& rd, const Register& rn, const Operand& operand);

    // Bitwise and (A & B) and update status flags.
    void ands(const Register& rd, const Register& rn, const Operand& operand);

    // Bit test, and set flags.
    void tst(const Register& rn, const Operand& operand);

    // Bit clear (A & ~B).
    void bic(const Register& rd, const Register& rn, const Operand& operand);

    // Bit clear (A & ~B) and update status flags.
    void bics(const Register& rd, const Register& rn, const Operand& operand);

    // Bitwise and.
    void and_(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Bit clear immediate.
    void bic(const VRegister& vd, const int imm8, const int left_shift = 0);

    // Bit clear.
    void bic(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Bitwise insert if false.
    void bif(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Bitwise insert if true.
    void bit(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Bitwise select.
    void bsl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // End of Instruction set functions --------------------------------------------------------------------------------
    
    void DataProcShiftedRegister(const Register& rd, const Register& rn, const Operand& operand, FlagsUpdate S,
                                 uint32_t op) {
        assert(operand.IsShiftedRegister());
        assert(rn.Is64Bits() || (rn.Is32Bits() && base::is_uint5(operand.shift_amount())));
        Emit(SF(rd) | op | Flags(S) | ShiftDP(operand.shift()) | ImmDPShift(operand.shift_amount()) | Rm(operand.reg()) |
             Rn(rn) | Rd(rd));
    }

    void DataProcExtendedRegister(const Register& rd, const Register& rn, const Operand& operand, FlagsUpdate S,
                                  uint32_t op) {
      uint32_t dest_reg = (S == SetFlags) ? Rd(rd) : RdSP(rd);
      Emit(SF(rd) | op | Flags(S) | Rm(operand.reg()) | ExtendMode(operand.extend()) |
           ImmExtendShift(operand.shift_amount()) | dest_reg | RnSP(rn));
    }
    
    void AddSub(const Register& rd, const Register& rn, const Operand& operand, FlagsUpdate S, AddSubOp op);
    
    int LinkAndGetInstructionOffsetTo(Label */*l*/) {
        // TODO:
        UNREACHABLE();
        return 0;
    }
    
    int LinkAndGetByteOffsetTo(Label */*l*/) {
        // TODO:
        UNREACHABLE();
        return 0;
    }
    
    const Register& AppropriateZeroRegFor(const CPURegister& reg) const { return reg.Is64Bits() ? xzr : wzr; }
    
    // Register encoding.
    static uint32_t Rd(CPURegister rd) {
        assert(rd.code() != kSPRegInternalCode);
        return rd.code() << Rd_offset;
    }

    static uint32_t Rn(CPURegister rn) {
        assert(rn.code() != kSPRegInternalCode);
        return rn.code() << Rn_offset;
    }

    static uint32_t Rm(CPURegister rm) {
        assert(rm.code() != kSPRegInternalCode);
        return rm.code() << Rm_offset;
    }

    static uint32_t RmNot31(CPURegister rm) {
        assert(rm.code() != kSPRegInternalCode);
        assert(!rm.IsZero());
        return Rm(rm);
    }

    static uint32_t Ra(CPURegister ra) {
        assert(ra.code() != kSPRegInternalCode);
        return ra.code() << Ra_offset;
    }

    static uint32_t Rt(CPURegister rt) {
        assert(rt.code() != kSPRegInternalCode);
        return rt.code() << Rt_offset;
    }

    static uint32_t Rt2(CPURegister rt2) {
        assert(rt2.code() != kSPRegInternalCode);
        return rt2.code() << Rt2_offset;
    }

    static uint32_t Rs(CPURegister rs) {
        assert(rs.code() != kSPRegInternalCode);
        return rs.code() << Rs_offset;
    }

    // These encoding functions allow the stack pointer to be encoded, and
    // disallow the zero register.
    static uint32_t RdSP(Register rd) {
        assert(!rd.IsZero());
        return (rd.code() & kRegCodeMask) << Rd_offset;
    }

    static uint32_t RnSP(Register rn) {
        assert(!rn.IsZero());
        return (rn.code() & kRegCodeMask) << Rn_offset;
    }
    
    // Flags encoding.
    static uint32_t Flags(FlagsUpdate S) {
        if (S == SetFlags) {
            return 1 << FlagsUpdate_offset;
        } else if (S == LeaveFlags) {
            return 0 << FlagsUpdate_offset;
        }
        UNREACHABLE();
    }
    
    static uint32_t Cond(Condition cond) { return cond << Condition_offset; }

    // PC-relative address encoding.
    inline static uint32_t ImmPCRelAddress(int imm21) {
        assert(base::is_int21(imm21));
        uint32_t imm = static_cast<uint32_t>(base::truncate_to_int21(imm21));
        uint32_t immhi = (imm >> ImmPCRelLo_width) << ImmPCRelHi_offset;
        uint32_t immlo = imm << ImmPCRelLo_offset;
        return (immhi & ImmPCRelHi_mask) | (immlo & ImmPCRelLo_mask);
    }

    // Branch encoding.
    static uint32_t ImmUncondBranch(int imm26) {
        assert(base::is_int26(imm26));
        return base::truncate_to_int26(imm26) << ImmUncondBranch_offset;
    }
    
    static uint32_t ImmCondBranch(int imm19) {
        assert(base::is_int19(imm19));
        return base::truncate_to_int19(imm19) << ImmCondBranch_offset;
    }
    
    static uint32_t ImmCmpBranch(int imm19) {
        assert(base::is_int19(imm19));
        return base::truncate_to_int19(imm19) << ImmCmpBranch_offset;
    }

    static uint32_t ImmTestBranch(int imm14) {
        assert(base::is_int14(imm14));
        return base::truncate_to_int14(imm14) << ImmTestBranch_offset;
    }

    static uint32_t ImmTestBranchBit(unsigned bit_pos) {
        assert(base::is_uint6(bit_pos));
        // Subtract five from the shift offset, as we need bit 5 from bit_pos.
        unsigned b5 = bit_pos << (ImmTestBranchBit5_offset - 5);
        unsigned b40 = bit_pos << ImmTestBranchBit40_offset;
        b5 &= ImmTestBranchBit5_mask;
        b40 &= ImmTestBranchBit40_mask;
        return b5 | b40;
    }

    // Data Processing encoding.
    static uint32_t SF(Register rd) { return rd.Is64Bits() ? SixtyFourBits : ThirtyTwoBits; }

    static uint32_t ImmAddSub(int imm) {
        assert(IsImmAddSub(imm));
        if (base::is_uint12(imm)) {  // No shift required.
            imm <<= ImmAddSub_offset;
        } else {
            imm = ((imm >> 12) << ImmAddSub_offset) | (1 << ShiftAddSub_offset);
        }
        return imm;
    }
    
    static uint32_t ImmS(unsigned imms, unsigned reg_size) {
        assert(((reg_size == kXRegSizeInBits) && base::is_uint6(imms)) ||
               ((reg_size == kWRegSizeInBits) && base::is_uint5(imms)));
        return imms << ImmS_offset;
    }

    static uint32_t ImmR(unsigned immr, unsigned reg_size) {
        assert(((reg_size == kXRegSizeInBits) && base::is_uint6(immr)) ||
               ((reg_size == kWRegSizeInBits) && base::is_uint5(immr)));
        //USE(reg_size);
        assert(base::is_uint6(immr));
        return immr << ImmR_offset;
    }

    static uint32_t ImmSetBits(unsigned imms, unsigned reg_size) {
        assert((reg_size == kWRegSizeInBits) || (reg_size == kXRegSizeInBits));
        assert(base::is_uint6(imms));
        assert((reg_size == kXRegSizeInBits) || base::is_uint6(imms + 3));
        //USE(reg_size);
        return imms << ImmSetBits_offset;
    }

    static uint32_t ImmRotate(unsigned immr, unsigned reg_size) {
        assert((reg_size == kWRegSizeInBits) || (reg_size == kXRegSizeInBits));
        assert(((reg_size == kXRegSizeInBits) && base::is_uint6(immr)) ||
               ((reg_size == kWRegSizeInBits) && base::is_uint5(immr)));
        //USE(reg_size);
        return immr << ImmRotate_offset;
    }
    
    static uint32_t ImmLLiteral(int imm19) {
        assert(base::is_int19(imm19));
        return base::truncate_to_int19(imm19) << ImmLLiteral_offset;
    }

    static uint32_t BitN(unsigned bitn, unsigned reg_size) {
        assert((reg_size == kWRegSizeInBits) || (reg_size == kXRegSizeInBits));
        assert((reg_size == kXRegSizeInBits) || (bitn == 0));
        //USE(reg_size);
        return bitn << BitN_offset;
    }

    static uint32_t ShiftDP(Shift shift) {
        assert(shift == LSL || shift == LSR || shift == ASR || shift == ROR);
        return shift << ShiftDP_offset;
    }

    static uint32_t ImmDPShift(unsigned amount) {
        assert(base::is_uint6(amount));
        return amount << ImmDPShift_offset;
    }

    static uint32_t ExtendMode(Extend extend) { return extend << ExtendMode_offset; }

    static uint32_t ImmExtendShift(unsigned left_shift) {
        assert(left_shift <= 4);
        return left_shift << ImmExtendShift_offset;
    }
    
    static uint32_t ImmCondCmp(unsigned imm) {
        assert(base::is_uint5(imm));
        return imm << ImmCondCmp_offset;
    }
    
    static uint32_t Nzcv(StatusFlags nzcv) { return ((nzcv >> Flags_offset) & 0xf) << Nzcv_offset; }

    static bool IsImmAddSub(int64_t immediate) {
        return base::is_uint12(immediate) || (base::is_uint12(immediate >> 12) && ((immediate & 0xFFF) == 0));
    }

    // Emit the instruction at pc_.
    void Emit(uint32_t instruction) {
        buf_.append(reinterpret_cast<char *>(&instruction), sizeof(instruction));
        pc_ += sizeof(instruction);
    }
    
    void EmitData(const void *data, size_t n) {
        buf_.append(static_cast<const char *>(data), n);
        pc_ += n;
    }
private:
    int pc_ = 0;
    std::string buf_;
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Assembler);
}; // class Assembler


} // namespace arm64

} // namespace yalx

#endif // YALX_ARM64_ASM_ARM64_H_
