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
    
    bool IsSameSizeAndType(const CPURegister& other) const {
        return (reg_size_ == other.reg_size_) && (type_ == other.type_);
    }

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


inline bool AreSameFormat(const VRegister& reg1, const VRegister& reg2, const VRegister& reg3 = NoVReg,
                          const VRegister& reg4 = NoVReg) {
    assert(reg1.is_valid());
    return (!reg2.is_valid() || reg2.IsSameFormat(reg1)) && (!reg3.is_valid() || reg3.IsSameFormat(reg1)) &&
           (!reg4.is_valid() || reg4.IsSameFormat(reg1));
}

inline bool AreSameSizeAndType(const CPURegister& reg1, const CPURegister& reg2,
                               const CPURegister& reg3 = NoVReg, const CPURegister& reg4 = NoVReg,
                               const CPURegister& reg5 = NoVReg, const CPURegister& reg6 = NoVReg,
                               const CPURegister& reg7 = NoVReg, const CPURegister& reg8 = NoVReg) {
    assert(reg1.is_valid());
    bool match = true;
    match &= !reg2.is_valid() || reg2.IsSameSizeAndType(reg1);
    match &= !reg3.is_valid() || reg3.IsSameSizeAndType(reg1);
    match &= !reg4.is_valid() || reg4.IsSameSizeAndType(reg1);
    match &= !reg5.is_valid() || reg5.IsSameSizeAndType(reg1);
    match &= !reg6.is_valid() || reg6.IsSameSizeAndType(reg1);
    match &= !reg7.is_valid() || reg7.IsSameSizeAndType(reg1);
    match &= !reg8.is_valid() || reg8.IsSameSizeAndType(reg1);
    return match;
}

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
        if (reg.IsSP()) { assert(shift_amount == 0); }
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
// MemOperand represents a memory operand in a load or store instruction.
class MemOperand {
public:
    MemOperand()
        : base_(NoReg)
        , regoffset_(NoReg)
        , offset_(0)
        , addrmode_(Offset)
        , shift_(NO_SHIFT)
        , extend_(NO_EXTEND)
        , shift_amount_(0) {}

    explicit MemOperand(Register base, int64_t offset = 0, AddrMode addrmode = Offset)
        : base_(base)
        , regoffset_(NoReg)
        , offset_(offset)
        , addrmode_(addrmode)
        , shift_(NO_SHIFT)
        , extend_(NO_EXTEND)
        , shift_amount_(0) {
            assert(base.Is64Bits() && !base.IsZero());
        }

    explicit MemOperand(Register base, Register regoffset, Shift shift = LSL, unsigned shift_amount = 0)
        : base_(base)
        , regoffset_(regoffset)
        , offset_(0)
        , addrmode_(Offset)
        , shift_(shift)
        , extend_(NO_EXTEND)
        , shift_amount_(shift_amount) {
        assert(base.Is64Bits() && !base.IsZero());
        assert(regoffset.Is64Bits() && !regoffset.IsSP());
        assert(shift == LSL);
    }
        
    explicit MemOperand(Register base, Register regoffset, Extend extend, unsigned shift_amount = 0)
        : base_(base)
        , regoffset_(regoffset)
        , offset_(0)
        , addrmode_(Offset)
        , shift_(NO_SHIFT)
        , extend_(extend)
        , shift_amount_(shift_amount) {
        assert(base.Is64Bits() && !base.IsZero());
        assert(!regoffset.IsSP());
        assert((extend == UXTW) || (extend == SXTW) || (extend == SXTX));

        // SXTX extend mode requires a 64-bit offset register.
        assert(regoffset.Is64Bits() || (extend != SXTX));
    }
    
    explicit MemOperand(Register base, const Operand& offset, AddrMode addrmode = Offset);
    
    DEF_VAL_GETTER(Register, base);
    DEF_VAL_GETTER(Register, regoffset);
    DEF_VAL_GETTER(int64_t, offset);
    DEF_VAL_GETTER(AddrMode, addrmode);
    DEF_VAL_GETTER(Shift, shift);
    DEF_VAL_GETTER(Extend, extend);
    DEF_VAL_GETTER(unsigned, shift_amount);
    
    bool IsImmediateOffset() const { return (addrmode_ == Offset) && regoffset_ == NoReg; }
    bool IsRegisterOffset() const { return (addrmode_ == Offset) && regoffset_ != NoReg; }
    bool IsPreIndex() const { return addrmode_ == PreIndex; }
    bool IsPostIndex() const { return addrmode_ == PostIndex; }

private:
    Register base_;
    Register regoffset_;
    int64_t offset_;
    AddrMode addrmode_;
    Shift shift_;
    Extend extend_;
    unsigned shift_amount_;
};


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
    void adc(const Register& rd, const Register& rn, const Operand& operand) {
        AddSubWithCarry(rd, rn, operand, LeaveFlags, ADC);
    }

    // Add with carry bit and update status flags.
    void adcs(const Register& rd, const Register& rn, const Operand& operand) {
        AddSubWithCarry(rd, rn, operand, SetFlags, ADC);
    }

    // Subtract with carry bit.
    void sbc(const Register& rd, const Register& rn, const Operand& operand) {
        AddSubWithCarry(rd, rn, operand, LeaveFlags, SBC);
    }

    // Subtract with carry bit and update status flags.
    void sbcs(const Register& rd, const Register& rn, const Operand& operand) {
        AddSubWithCarry(rd, rn, operand, SetFlags, SBC);
    }

    // Negate with carry bit.
    void ngc(const Register& rd, const Operand& operand) {
        Register zr = AppropriateZeroRegFor(rd);
        sbc(rd, zr, operand);
    }

    // Negate with carry bit and update status flags.
    void ngcs(const Register& rd, const Operand& operand) {
        Register zr = AppropriateZeroRegFor(rd);
        sbcs(rd, zr, operand);
    }

    // Logical instructions.
    // Bitwise and (A & B).
    void and_(const Register& rd, const Register& rn, const Operand& operand) { Logical(rd, rn, operand, AND); }

    // Bitwise and (A & B) and update status flags.
    void ands(const Register& rd, const Register& rn, const Operand& operand) { Logical(rd, rn, operand, ANDS); }

    // Bit test, and set flags.
    void tst(const Register& rn, const Operand& operand) { ands(AppropriateZeroRegFor(rn), rn, operand); }

    // Bit clear (A & ~B).
    void bic(const Register& rd, const Register& rn, const Operand& operand) { Logical(rd, rn, operand, BIC); }

    // Bit clear (A & ~B) and update status flags.
    void bics(const Register& rd, const Register& rn, const Operand& operand) { Logical(rd, rn, operand, BICS); }

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

    // Polynomial multiply.
    void pmul(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Vector move immediate.
    void movi(const VRegister& vd, const uint64_t imm, Shift shift = LSL,
              const int shift_amount = 0);

    // Bitwise not.
    void mvn(const VRegister& vd, const VRegister& vn);

    // Vector move inverted immediate.
    void mvni(const VRegister& vd, const int imm8, Shift shift = LSL,
              const int shift_amount = 0);

    // Signed saturating accumulate of unsigned value.
    void suqadd(const VRegister& vd, const VRegister& vn);

    // Unsigned saturating accumulate of signed value.
    void usqadd(const VRegister& vd, const VRegister& vn);

    // Absolute value.
    void abs(const VRegister& vd, const VRegister& vn);

    // Signed saturating absolute value.
    void sqabs(const VRegister& vd, const VRegister& vn);

    // Negate.
    void neg(const VRegister& vd, const VRegister& vn);

    // Signed saturating negate.
    void sqneg(const VRegister& vd, const VRegister& vn);

    // Bitwise not.
    void not_(const VRegister& vd, const VRegister& vn);

    // Extract narrow.
    void xtn(const VRegister& vd, const VRegister& vn);

    // Extract narrow (second part).
    void xtn2(const VRegister& vd, const VRegister& vn);

    // Signed saturating extract narrow.
    void sqxtn(const VRegister& vd, const VRegister& vn);

    // Signed saturating extract narrow (second part).
    void sqxtn2(const VRegister& vd, const VRegister& vn);

    // Unsigned saturating extract narrow.
    void uqxtn(const VRegister& vd, const VRegister& vn);

    // Unsigned saturating extract narrow (second part).
    void uqxtn2(const VRegister& vd, const VRegister& vn);

    // Signed saturating extract unsigned narrow.
    void sqxtun(const VRegister& vd, const VRegister& vn);

    // Signed saturating extract unsigned narrow (second part).
    void sqxtun2(const VRegister& vd, const VRegister& vn);

    // Move register to register.
    void mov(const VRegister& vd, const VRegister& vn);

    // Bitwise not or.
    void orn(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Bitwise exclusive or.
    void eor(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Bitwise or (A | B).
    void orr(const Register& rd, const Register& rn, const Operand& operand) { Logical(rd, rn, operand, ORR); }

    // Bitwise or.
    void orr(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Bitwise or immediate.
    void orr(const VRegister& vd, const int imm8, const int left_shift = 0);
    
    // Bitwise nor (A | ~B).
    void orn(const Register& rd, const Register& rn, const Operand& operand) { Logical(rd, rn, operand, ORN); }

    // Bitwise eor/xor (A ^ B).
    void eor(const Register& rd, const Register& rn, const Operand& operand) { Logical(rd, rn, operand, EOR); }

    // Bitwise enor/xnor (A ^ ~B).
    void eon(const Register& rd, const Register& rn, const Operand& operand) { Logical(rd, rn, operand, EON); }

    // Logical shift left variable.
    void lslv(const Register& rd, const Register& rn, const Register& rm) {
        assert(rd.size_in_bits() == rn.size_in_bits());
        assert(rd.size_in_bits() == rm.size_in_bits());
        Emit(SF(rd) | LSLV | Rm(rm) | Rn(rn) | Rd(rd));
    }

    // Logical shift right variable.
    void lsrv(const Register& rd, const Register& rn, const Register& rm) {
        assert(rd.size_in_bits() == rn.size_in_bits());
        assert(rd.size_in_bits() == rm.size_in_bits());
        Emit(SF(rd) | LSRV | Rm(rm) | Rn(rn) | Rd(rd));
    }

    // Arithmetic shift right variable.
    void asrv(const Register& rd, const Register& rn, const Register& rm) {
        assert(rd.size_in_bits() == rn.size_in_bits());
        assert(rd.size_in_bits() == rm.size_in_bits());
        Emit(SF(rd) | ASRV | Rm(rm) | Rn(rn) | Rd(rd));
    }

    // Rotate right variable.
    void rorv(const Register& rd, const Register& rn, const Register& rm) {
        assert(rd.size_in_bits() == rn.size_in_bits());
        assert(rd.size_in_bits() == rm.size_in_bits());
        Emit(SF(rd) | RORV | Rm(rm) | Rn(rn) | Rd(rd));
    }

    // Bitfield instructions.
    // Bitfield move.
    void bfm(const Register& rd, const Register& rn, int immr, int imms) {
        assert(rd.size_in_bits() == rn.size_in_bits());
        uint32_t N = SF(rd) >> (kSFOffset - kBitfieldNOffset);
        Emit(SF(rd) | BFM | N | ImmR(immr, rd.size_in_bits()) | ImmS(imms, rn.size_in_bits()) | Rn(rn) | Rd(rd));
    }

    // Signed bitfield move.
    void sbfm(const Register& rd, const Register& rn, int immr, int imms) {
        assert(rd.Is64Bits() || rn.Is32Bits());
        uint32_t N = SF(rd) >> (kSFOffset - kBitfieldNOffset);
        Emit(SF(rd) | SBFM | N | ImmR(immr, rd.size_in_bits()) | ImmS(imms, rn.size_in_bits()) | Rn(rn) | Rd(rd));
    }

    // Unsigned bitfield move.
    void ubfm(const Register& rd, const Register& rn, int immr, int imms) {
        assert(rd.size_in_bits() == rn.size_in_bits());
        uint32_t N = SF(rd) >> (kSFOffset - kBitfieldNOffset);
        Emit(SF(rd) | UBFM | N | ImmR(immr, rd.size_in_bits()) | ImmS(imms, rn.size_in_bits()) | Rn(rn) | Rd(rd));
    }

    // Bfm aliases.
    // Bitfield insert.
    void bfi(const Register& rd, const Register& rn, int lsb, int width) {
        assert(width >= 1);
        assert(lsb + width <= rn.size_in_bits());
        bfm(rd, rn, (rd.size_in_bits() - lsb) & (rd.size_in_bits() - 1), width - 1);
    }

    // Bitfield extract and insert low.
    void bfxil(const Register& rd, const Register& rn, int lsb, int width) {
        assert(width >= 1);
        assert(lsb + width <= rn.size_in_bits());
        bfm(rd, rn, lsb, lsb + width - 1);
    }

    // Sbfm aliases.
    // Arithmetic shift right.
    void asr(const Register& rd, const Register& rn, int shift) {
        assert(shift < rd.size_in_bits());
        sbfm(rd, rn, shift, rd.size_in_bits() - 1);
    }

    // Signed bitfield insert in zero.
    void sbfiz(const Register& rd, const Register& rn, int lsb, int width) {
        assert(width >= 1);
        assert(lsb + width <= rn.size_in_bits());
        sbfm(rd, rn, (rd.size_in_bits() - lsb) & (rd.size_in_bits() - 1), width - 1);
    }

    // Signed bitfield extract.
    void sbfx(const Register& rd, const Register& rn, int lsb, int width) {
        assert(width >= 1);
        assert(lsb + width <= rn.size_in_bits());
        sbfm(rd, rn, lsb, lsb + width - 1);
    }

    // Signed extend byte.
    void sxtb(const Register& rd, const Register& rn) { sbfm(rd, rn, 0, 7); }

    // Signed extend halfword.
    void sxth(const Register& rd, const Register& rn) { sbfm(rd, rn, 0, 15); }

    // Signed extend word.
    void sxtw(const Register& rd, const Register& rn) { sbfm(rd, rn, 0, 31); }

    // Ubfm aliases.
    // Logical shift left.
    void lsl(const Register& rd, const Register& rn, int shift) {
        int reg_size = rd.size_in_bits();
        assert(shift < reg_size);
        ubfm(rd, rn, (reg_size - shift) % reg_size, reg_size - shift - 1);
    }

    // Logical shift right.
    void lsr(const Register& rd, const Register& rn, int shift) {
        assert(shift < rd.size_in_bits());
        ubfm(rd, rn, shift, rd.size_in_bits() - 1);
    }

    // Unsigned bitfield insert in zero.
    void ubfiz(const Register& rd, const Register& rn, int lsb, int width) {
        assert(width >= 1);
        assert(lsb + width <= rn.size_in_bits());
        ubfm(rd, rn, (rd.size_in_bits() - lsb) & (rd.size_in_bits() - 1), width - 1);
    }

    // Unsigned bitfield extract.
    void ubfx(const Register& rd, const Register& rn, int lsb, int width) {
        assert(width >= 1);
        assert(lsb + width <= rn.size_in_bits());
        ubfm(rd, rn, lsb, lsb + width - 1);
    }

    // Unsigned extend byte.
    void uxtb(const Register& rd, const Register& rn) { ubfm(rd, rn, 0, 7); }

    // Unsigned extend halfword.
    void uxth(const Register& rd, const Register& rn) { ubfm(rd, rn, 0, 15); }

    // Unsigned extend word.
    void uxtw(const Register& rd, const Register& rn) { ubfm(rd, rn, 0, 31); }

    // Extract.
    void extr(const Register& rd, const Register& rn, const Register& rm, int lsb) {
        assert(rd.size_in_bits() == rn.size_in_bits());
        assert(rd.size_in_bits() == rm.size_in_bits());
        uint32_t N = SF(rd) >> (kSFOffset - kBitfieldNOffset);
        Emit(SF(rd) | EXTR | N | Rm(rm) | ImmS(lsb, rn.size_in_bits()) | Rn(rn) | Rd(rd));
    }

    // Conditional select: rd = cond ? rn : rm.
    void csel(const Register& rd, const Register& rn, const Register& rm, Condition cond) {
        ConditionalSelect(rd, rn, rm, cond, CSEL);
    }

    // Conditional select increment: rd = cond ? rn : rm + 1.
    void csinc(const Register& rd, const Register& rn, const Register& rm, Condition cond) {
        ConditionalSelect(rd, rn, rm, cond, CSINC);
    }

    // Conditional select inversion: rd = cond ? rn : ~rm.
    void csinv(const Register& rd, const Register& rn, const Register& rm, Condition cond) {
        ConditionalSelect(rd, rn, rm, cond, CSINV);
    }

    // Conditional select negation: rd = cond ? rn : -rm.
    void csneg(const Register& rd, const Register& rn, const Register& rm, Condition cond) {
        ConditionalSelect(rd, rn, rm, cond, CSNEG);
    }

    // Conditional set: rd = cond ? 1 : 0.
    void cset(const Register& rd, Condition cond) {
        assert((cond != al) && (cond != nv));
        Register zr = AppropriateZeroRegFor(rd);
        csinc(rd, zr, zr, NegateCondition(cond));
    }

    // Conditional set minus: rd = cond ? -1 : 0.
    void csetm(const Register& rd, Condition cond) {
        assert((cond != al) && (cond != nv));
        Register zr = AppropriateZeroRegFor(rd);
        csinv(rd, zr, zr, NegateCondition(cond));
    }

    // Conditional increment: rd = cond ? rn + 1 : rn.
    void cinc(const Register& rd, const Register& rn, Condition cond) {
        assert((cond != al) && (cond != nv));
        csinc(rd, rn, rn, NegateCondition(cond));
    }

    // Conditional invert: rd = cond ? ~rn : rn.
    void cinv(const Register& rd, const Register& rn, Condition cond) {
        assert((cond != al) && (cond != nv));
        csinv(rd, rn, rn, NegateCondition(cond));
    }

    // Conditional negate: rd = cond ? -rn : rn.
    void cneg(const Register& rd, const Register& rn, Condition cond) {
        assert((cond != al) && (cond != nv));
        csneg(rd, rn, rn, NegateCondition(cond));
    }

    // Extr aliases.
    void ror(const Register& rd, const Register& rs, unsigned shift) { extr(rd, rs, rs, shift); }

    // Conditional comparison.
    // Conditional compare negative.
    void ccmn(const Register& rn, const Operand& operand, StatusFlags nzcv, Condition cond) {
        ConditionalCompare(rn, operand, nzcv, cond, CCMN);
    }

    // Conditional compare.
    void ccmp(const Register& rn, const Operand& operand, StatusFlags nzcv, Condition cond) {
        ConditionalCompare(rn, operand, nzcv, cond, CCMP);
    }

    // Multiplication.
    // 32 x 32 -> 32-bit and 64 x 64 -> 64-bit multiply.
    void mul(const Register& rd, const Register& rn, const Register& rm) {
        assert(AreSameSizeAndType(rd, rn, rm));
        Register zr = AppropriateZeroRegFor(rn);
        DataProcessing3Source(rd, rn, rm, zr, MADD);
    }

    // 32 + 32 x 32 -> 32-bit and 64 + 64 x 64 -> 64-bit multiply accumulate.
    void madd(const Register& rd, const Register& rn, const Register& rm, const Register& ra) {
        assert(AreSameSizeAndType(rd, rn, rm, ra));
        DataProcessing3Source(rd, rn, rm, ra, MADD);
    }

    // -(32 x 32) -> 32-bit and -(64 x 64) -> 64-bit multiply.
    void mneg(const Register& rd, const Register& rn, const Register& rm) {
        assert(AreSameSizeAndType(rd, rn, rm));
        Register zr = AppropriateZeroRegFor(rn);
        DataProcessing3Source(rd, rn, rm, zr, MSUB);
    }

    // 32 - 32 x 32 -> 32-bit and 64 - 64 x 64 -> 64-bit multiply subtract.
    void msub(const Register& rd, const Register& rn, const Register& rm, const Register& ra) {
        assert(AreSameSizeAndType(rd, rn, rm, ra));
        DataProcessing3Source(rd, rn, rm, ra, MSUB);
    }

    // 32 x 32 -> 64-bit multiply.
    void smull(const Register& rd, const Register& rn, const Register& rm) {
        assert(rd.Is64Bits());
        assert(rn.Is32Bits() && rm.Is32Bits());
        DataProcessing3Source(rd, rn, rm, xzr, SMADDL_x);
    }

    // Xd = bits<127:64> of Xn * Xm.
    void smulh(const Register& rd, const Register& rn, const Register& rm) {
        assert(AreSameSizeAndType(rd, rn, rm));
        DataProcessing3Source(rd, rn, rm, xzr, SMULH_x);
    }

    // Signed 32 x 32 -> 64-bit multiply and accumulate.
    void smaddl(const Register& rd, const Register& rn, const Register& rm,
                const Register& ra) {
        assert(rd.Is64Bits() && ra.Is64Bits());
        assert(rn.Is32Bits() && rm.Is32Bits());
        DataProcessing3Source(rd, rn, rm, ra, SMADDL_x);
    }

    // Unsigned 32 x 32 -> 64-bit multiply and accumulate.
    void umaddl(const Register& rd, const Register& rn, const Register& rm,
                const Register& ra) {
        assert(rd.Is64Bits() && ra.Is64Bits());
        assert(rn.Is32Bits() && rm.Is32Bits());
        DataProcessing3Source(rd, rn, rm, ra, UMADDL_x);
    }

    // Signed 32 x 32 -> 64-bit multiply and subtract.
    void smsubl(const Register& rd, const Register& rn, const Register& rm,
                const Register& ra) {
        assert(rd.Is64Bits() && ra.Is64Bits());
        assert(rn.Is32Bits() && rm.Is32Bits());
        DataProcessing3Source(rd, rn, rm, ra, SMSUBL_x);
    }

    // Unsigned 32 x 32 -> 64-bit multiply and subtract.
    void umsubl(const Register& rd, const Register& rn, const Register& rm,
                const Register& ra) {
        assert(rd.Is64Bits() && ra.Is64Bits());
        assert(rn.Is32Bits() && rm.Is32Bits());
        DataProcessing3Source(rd, rn, rm, ra, UMSUBL_x);
    }

    // Signed integer divide.
    void sdiv(const Register& rd, const Register& rn, const Register& rm) {
        assert(rd.size_in_bits() == rn.size_in_bits());
        assert(rd.size_in_bits() == rm.size_in_bits());
        Emit(SF(rd) | SDIV | Rm(rm) | Rn(rn) | Rd(rd));
    }

    // Unsigned integer divide.
    void udiv(const Register& rd, const Register& rn, const Register& rm) {
        assert(rd.size_in_bits() == rn.size_in_bits());
        assert(rd.size_in_bits() == rm.size_in_bits());
        Emit(SF(rd) | UDIV | Rm(rm) | Rn(rn) | Rd(rd));
    }

    // Bit count, bit reverse and endian reverse.
    void rbit(const Register& rd, const Register& rn) { DataProcessing1Source(rd, rn, RBIT); }
    void rev16(const Register& rd, const Register& rn) { DataProcessing1Source(rd, rn, REV16); }
    void rev32(const Register& rd, const Register& rn) {
        assert(rd.Is64Bits());
        DataProcessing1Source(rd, rn, REV);
    }
    void rev(const Register& rd, const Register& rn) { DataProcessing1Source(rd, rn, rd.Is64Bits() ? REV_x : REV_w); }
    void clz(const Register& rd, const Register& rn) { DataProcessing1Source(rd, rn, CLZ); }
    void cls(const Register& rd, const Register& rn) { DataProcessing1Source(rd, rn, CLS); }

    // Pointer Authentication Code for Instruction address, using key B, with
    // address in x17 and modifier in x16 [Armv8.3].
    void pacib1716() { Emit(PACIB1716); }

    // Pointer Authentication Code for Instruction address, using key B, with
    // address in LR and modifier in SP [Armv8.3].
    void pacibsp()  { Emit(PACIBSP); }

    // Authenticate Instruction address, using key B, with address in x17 and
    // modifier in x16 [Armv8.3].
    void autib1716() { Emit(AUTIB1716); }

    // Authenticate Instruction address, using key B, with address in LR and
    // modifier in SP [Armv8.3].
    void autibsp() { Emit(AUTIBSP); }

    // Memory instructions.

    // Load integer or FP register.
    void ldr(const CPURegister& rt, const MemOperand& src) { LoadStore(rt, src, LoadOpFor(rt)); }

    // Store integer or FP register.
    void str(const CPURegister& rt, const MemOperand& dst) { LoadStore(rt, dst, StoreOpFor(rt)); }

    // Load word with sign extension.
    void ldrsw(const Register& rt, const MemOperand& src) {
        assert(rt.Is64Bits());
        LoadStore(rt, src, LDRSW_x);
    }

    // Load byte.
    void ldrb(const Register& rt, const MemOperand& src) { LoadStore(rt, src, LDRB_w); }

    // Store byte.
    void strb(const Register& rt, const MemOperand& dst) { LoadStore(rt, dst, STRB_w); }

    // Load byte with sign extension.
    void ldrsb(const Register& rt, const MemOperand& src) { LoadStore(rt, src, rt.Is64Bits() ? LDRSB_x : LDRSB_w); }

    // Load half-word.
    void ldrh(const Register& rt, const MemOperand& src) { LoadStore(rt, src, LDRH_w); }

    // Store half-word.
    void strh(const Register& rt, const MemOperand& dst) { LoadStore(rt, dst, STRH_w); }

    // Load half-word with sign extension.
    void ldrsh(const Register& rt, const MemOperand& src) { LoadStore(rt, src, rt.Is64Bits() ? LDRSH_x : LDRSH_w); }

    // Load integer or FP register pair.
    void ldp(const CPURegister& rt, const CPURegister& rt2, const MemOperand& src);

    // Store integer or FP register pair.
    void stp(const CPURegister& rt, const CPURegister& rt2,
             const MemOperand& dst);

    // Load word pair with sign extension.
    void ldpsw(const Register& rt, const Register& rt2, const MemOperand& src);

    // Load literal to register from a pc relative address.
    void ldr_pcrel(const CPURegister& rt, int imm19);

    // Load literal to register.
    //void ldr(const CPURegister& rt, const Immediate& imm);
    void ldr(const CPURegister& rt, const Operand& operand);

    // Load-acquire word.
    void ldar(const Register& rt, const Register& rn);

    // Load-acquire exclusive word.
    void ldaxr(const Register& rt, const Register& rn);

    // Store-release word.
    void stlr(const Register& rt, const Register& rn);

    // Store-release exclusive word.
    void stlxr(const Register& rs, const Register& rt, const Register& rn);

    // Load-acquire byte.
    void ldarb(const Register& rt, const Register& rn);

    // Load-acquire exclusive byte.
    void ldaxrb(const Register& rt, const Register& rn);

    // Store-release byte.
    void stlrb(const Register& rt, const Register& rn);

    // Store-release exclusive byte.
    void stlxrb(const Register& rs, const Register& rt, const Register& rn);

    // Load-acquire half-word.
    void ldarh(const Register& rt, const Register& rn);

    // Load-acquire exclusive half-word.
    void ldaxrh(const Register& rt, const Register& rn);

    // Store-release half-word.
    void stlrh(const Register& rt, const Register& rn);

    // Store-release exclusive half-word.
    void stlxrh(const Register& rs, const Register& rt, const Register& rn);

    // Move instructions. The default shift of -1 indicates that the move
    // instruction will calculate an appropriate 16-bit immediate and left shift
    // that is equal to the 64-bit immediate argument. If an explicit left shift
    // is specified (0, 16, 32 or 48), the immediate must be a 16-bit value.
    //
    // For movk, an explicit shift can be used to indicate which half word should
    // be overwritten, eg. movk(x0, 0, 0) will overwrite the least-significant
    // half word with zero, whereas movk(x0, 0, 48) will overwrite the
    // most-significant.

    // Move and keep.
    void movk(const Register& rd, uint64_t imm, int shift = -1) {
      //MoveWide(rd, imm, shift, MOVK);
    }

    // Move with non-zero.
    void movn(const Register& rd, uint64_t imm, int shift = -1) {
      //MoveWide(rd, imm, shift, MOVN);
    }

    // Move with zero.
    void movz(const Register& rd, uint64_t imm, int shift = -1) {
      //MoveWide(rd, imm, shift, MOVZ);
    }

    // Misc instructions.
    // Monitor debug-mode breakpoint.
    void brk(int code);

    // Halting debug-mode breakpoint.
    void hlt(int code);

    // Move register to register.
    void mov(const Register& rd, const Register& rn);

    // Move NOT(operand) to register.
    void mvn(const Register& rd, const Operand& operand);

    // System instructions.
    // Move to register from system register.
    //void mrs(const Register& rt, SystemRegister sysreg);

    // Move from register to system register.
    //void msr(SystemRegister sysreg, const Register& rt);

    // System hint.
    void hint(SystemHint code);

    // Data memory barrier
    void dmb(BarrierDomain domain, BarrierType type);

    // Data synchronization barrier
    void dsb(BarrierDomain domain, BarrierType type);

    // Instruction synchronization barrier
    void isb();

    // Conditional speculation barrier.
    void csdb();

    // Branch target identification.
    void bti(BranchTargetIdentifier id) {
        SystemHint op;
        switch (id) {
          case BranchTargetIdentifier::kBti:
            op = BTI;
            break;
          case BranchTargetIdentifier::kBtiCall:
            op = BTI_c;
            break;
          case BranchTargetIdentifier::kBtiJump:
            op = BTI_j;
            break;
          case BranchTargetIdentifier::kBtiJumpCall:
            op = BTI_jc;
            break;
          case BranchTargetIdentifier::kNone:
          case BranchTargetIdentifier::kPacibsp:
            // We always want to generate a BTI instruction here, so disallow
            // skipping its generation or generating a PACIBSP instead.
            UNREACHABLE();
        }
        hint(op);
    }

    // No-op.
    void nop() { hint(NOP); }
    
    // Add.
    void add(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned halving add.
    void uhadd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Subtract.
    void sub(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed halving add.
    void shadd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Multiply by scalar element.
    void mul(const VRegister& vd, const VRegister& vn, const VRegister& vm, int vm_index);

    // Multiply-add by scalar element.
    void mla(const VRegister& vd, const VRegister& vn, const VRegister& vm,
             int vm_index);

    // Multiply-subtract by scalar element.
    void mls(const VRegister& vd, const VRegister& vn, const VRegister& vm,
             int vm_index);

    // Signed long multiply-add by scalar element.
    void smlal(const VRegister& vd, const VRegister& vn, const VRegister& vm,
               int vm_index);

    // Signed long multiply-add by scalar element (second part).
    void smlal2(const VRegister& vd, const VRegister& vn, const VRegister& vm,
                int vm_index);

    // Unsigned long multiply-add by scalar element.
    void umlal(const VRegister& vd, const VRegister& vn, const VRegister& vm,
               int vm_index);

    // Unsigned long multiply-add by scalar element (second part).
    void umlal2(const VRegister& vd, const VRegister& vn, const VRegister& vm,
                int vm_index);

    // Signed long multiply-sub by scalar element.
    void smlsl(const VRegister& vd, const VRegister& vn, const VRegister& vm,
               int vm_index);

    // Signed long multiply-sub by scalar element (second part).
    void smlsl2(const VRegister& vd, const VRegister& vn, const VRegister& vm,
                int vm_index);

    // Unsigned long multiply-sub by scalar element.
    void umlsl(const VRegister& vd, const VRegister& vn, const VRegister& vm,
               int vm_index);

    // Unsigned long multiply-sub by scalar element (second part).
    void umlsl2(const VRegister& vd, const VRegister& vn, const VRegister& vm,
                int vm_index);

    // Signed long multiply by scalar element.
    void smull(const VRegister& vd, const VRegister& vn, const VRegister& vm,
               int vm_index);

    // Signed long multiply by scalar element (second part).
    void smull2(const VRegister& vd, const VRegister& vn, const VRegister& vm,
                int vm_index);

    // Unsigned long multiply by scalar element.
    void umull(const VRegister& vd, const VRegister& vn, const VRegister& vm,
               int vm_index);

    // Unsigned long multiply by scalar element (second part).
    void umull2(const VRegister& vd, const VRegister& vn, const VRegister& vm,
                int vm_index);

    // Add narrow returning high half.
    void addhn(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Add narrow returning high half (second part).
    void addhn2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed saturating double long multiply by element.
    void sqdmull(const VRegister& vd, const VRegister& vn, const VRegister& vm,
                 int vm_index);

    // Signed saturating double long multiply by element (second part).
    void sqdmull2(const VRegister& vd, const VRegister& vn, const VRegister& vm,
                  int vm_index);

    // Signed saturating doubling long multiply-add by element.
    void sqdmlal(const VRegister& vd, const VRegister& vn, const VRegister& vm,
                 int vm_index);

    // Signed saturating doubling long multiply-add by element (second part).
    void sqdmlal2(const VRegister& vd, const VRegister& vn, const VRegister& vm,
                  int vm_index);

    // Signed saturating doubling long multiply-sub by element.
    void sqdmlsl(const VRegister& vd, const VRegister& vn, const VRegister& vm,
                 int vm_index);

    // Signed saturating doubling long multiply-sub by element (second part).
    void sqdmlsl2(const VRegister& vd, const VRegister& vn, const VRegister& vm,
                  int vm_index);

    // Compare bitwise to zero.
    void cmeq(const VRegister& vd, const VRegister& vn, int value);

    // Compare signed greater than or equal to zero.
    void cmge(const VRegister& vd, const VRegister& vn, int value);

    // Compare signed greater than zero.
    void cmgt(const VRegister& vd, const VRegister& vn, int value);

    // Compare signed less than or equal to zero.
    void cmle(const VRegister& vd, const VRegister& vn, int value);

    // Compare signed less than zero.
    void cmlt(const VRegister& vd, const VRegister& vn, int value);

    // Unsigned rounding halving add.
    void urhadd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Compare equal.
    void cmeq(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Compare signed greater than or equal.
    void cmge(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Compare signed greater than.
    void cmgt(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Compare unsigned higher.
    void cmhi(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Compare unsigned higher or same.
    void cmhs(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Compare bitwise test bits nonzero.
    void cmtst(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed shift left by register.
    void sshl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned shift left by register.
    void ushl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed saturating doubling long multiply-subtract.
    void sqdmlsl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed saturating doubling long multiply-subtract (second part).
    void sqdmlsl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed saturating doubling long multiply.
    void sqdmull(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed saturating doubling long multiply (second part).
    void sqdmull2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed saturating doubling multiply returning high half.
    void sqdmulh(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed saturating rounding doubling multiply returning high half.
    void sqrdmulh(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed saturating doubling multiply element returning high half.
    void sqdmulh(const VRegister& vd, const VRegister& vn, const VRegister& vm,
                 int vm_index);

    // Signed saturating rounding doubling multiply element returning high half.
    void sqrdmulh(const VRegister& vd, const VRegister& vn, const VRegister& vm,
                  int vm_index);

    // Unsigned long multiply long.
    void umull(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned long multiply (second part).
    void umull2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Rounding add narrow returning high half.
    void raddhn(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Subtract narrow returning high half.
    void subhn(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Subtract narrow returning high half (second part).
    void subhn2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Rounding add narrow returning high half (second part).
    void raddhn2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Rounding subtract narrow returning high half.
    void rsubhn(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Rounding subtract narrow returning high half (second part).
    void rsubhn2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed saturating shift left by register.
    void sqshl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned saturating shift left by register.
    void uqshl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed rounding shift left by register.
    void srshl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned rounding shift left by register.
    void urshl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed saturating rounding shift left by register.
    void sqrshl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned saturating rounding shift left by register.
    void uqrshl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed absolute difference.
    void sabd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned absolute difference and accumulate.
    void uaba(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Shift left by immediate and insert.
    void sli(const VRegister& vd, const VRegister& vn, int shift);

    // Shift right by immediate and insert.
    void sri(const VRegister& vd, const VRegister& vn, int shift);

    // Signed maximum.
    void smax(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed pairwise maximum.
    void smaxp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Add across vector.
    void addv(const VRegister& vd, const VRegister& vn);

    // Signed add long across vector.
    void saddlv(const VRegister& vd, const VRegister& vn);

    // Unsigned add long across vector.
    void uaddlv(const VRegister& vd, const VRegister& vn);

    // FP maximum number across vector.
    void fmaxnmv(const VRegister& vd, const VRegister& vn);

    // FP maximum across vector.
    void fmaxv(const VRegister& vd, const VRegister& vn);

    // FP minimum number across vector.
    void fminnmv(const VRegister& vd, const VRegister& vn);

    // FP minimum across vector.
    void fminv(const VRegister& vd, const VRegister& vn);

    // Signed maximum across vector.
    void smaxv(const VRegister& vd, const VRegister& vn);

    // Signed minimum.
    void smin(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed minimum pairwise.
    void sminp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed minimum across vector.
    void sminv(const VRegister& vd, const VRegister& vn);

    // One-element structure store from one register.
    void st1(const VRegister& vt, const MemOperand& src);

    // One-element structure store from two registers.
    void st1(const VRegister& vt, const VRegister& vt2, const MemOperand& src);

    // One-element structure store from three registers.
    void st1(const VRegister& vt, const VRegister& vt2, const VRegister& vt3,
             const MemOperand& src);

    // One-element structure store from four registers.
    void st1(const VRegister& vt, const VRegister& vt2, const VRegister& vt3,
             const VRegister& vt4, const MemOperand& src);

    // One-element single structure store from one lane.
    void st1(const VRegister& vt, int lane, const MemOperand& src);

    // Two-element structure store from two registers.
    void st2(const VRegister& vt, const VRegister& vt2, const MemOperand& src);

    // Two-element single structure store from two lanes.
    void st2(const VRegister& vt, const VRegister& vt2, int lane,
             const MemOperand& src);

    // Three-element structure store from three registers.
    void st3(const VRegister& vt, const VRegister& vt2, const VRegister& vt3,
             const MemOperand& src);

    // Three-element single structure store from three lanes.
    void st3(const VRegister& vt, const VRegister& vt2, const VRegister& vt3,
             int lane, const MemOperand& src);

    // Four-element structure store from four registers.
    void st4(const VRegister& vt, const VRegister& vt2, const VRegister& vt3,
             const VRegister& vt4, const MemOperand& src);

    // Four-element single structure store from four lanes.
    void st4(const VRegister& vt, const VRegister& vt2, const VRegister& vt3,
             const VRegister& vt4, int lane, const MemOperand& src);

    // Unsigned add long.
    void uaddl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned add long (second part).
    void uaddl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned add wide.
    void uaddw(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned add wide (second part).
    void uaddw2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed add long.
    void saddl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed add long (second part).
    void saddl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed add wide.
    void saddw(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed add wide (second part).
    void saddw2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned subtract long.
    void usubl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned subtract long (second part).
    void usubl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned subtract wide.
    void usubw(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed subtract long.
    void ssubl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed subtract long (second part).
    void ssubl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed integer subtract wide.
    void ssubw(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed integer subtract wide (second part).
    void ssubw2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned subtract wide (second part).
    void usubw2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned maximum.
    void umax(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned pairwise maximum.
    void umaxp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned maximum across vector.
    void umaxv(const VRegister& vd, const VRegister& vn);

    // Unsigned minimum.
    void umin(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned pairwise minimum.
    void uminp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned minimum across vector.
    void uminv(const VRegister& vd, const VRegister& vn);

    // Transpose vectors (primary).
    void trn1(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Transpose vectors (secondary).
    void trn2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unzip vectors (primary).
    void uzp1(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unzip vectors (secondary).
    void uzp2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Zip vectors (primary).
    void zip1(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Zip vectors (secondary).
    void zip2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed shift right by immediate.
    void sshr(const VRegister& vd, const VRegister& vn, int shift);

    // Unsigned shift right by immediate.
    void ushr(const VRegister& vd, const VRegister& vn, int shift);

    // Signed rounding shift right by immediate.
    void srshr(const VRegister& vd, const VRegister& vn, int shift);

    // Unsigned rounding shift right by immediate.
    void urshr(const VRegister& vd, const VRegister& vn, int shift);

    // Signed shift right by immediate and accumulate.
    void ssra(const VRegister& vd, const VRegister& vn, int shift);

    // Unsigned shift right by immediate and accumulate.
    void usra(const VRegister& vd, const VRegister& vn, int shift);

    // Signed rounding shift right by immediate and accumulate.
    void srsra(const VRegister& vd, const VRegister& vn, int shift);

    // Unsigned rounding shift right by immediate and accumulate.
    void ursra(const VRegister& vd, const VRegister& vn, int shift);

    // Shift right narrow by immediate.
    void shrn(const VRegister& vd, const VRegister& vn, int shift);

    // Shift right narrow by immediate (second part).
    void shrn2(const VRegister& vd, const VRegister& vn, int shift);

    // Rounding shift right narrow by immediate.
    void rshrn(const VRegister& vd, const VRegister& vn, int shift);

    // Rounding shift right narrow by immediate (second part).
    void rshrn2(const VRegister& vd, const VRegister& vn, int shift);

    // Unsigned saturating shift right narrow by immediate.
    void uqshrn(const VRegister& vd, const VRegister& vn, int shift);

    // Unsigned saturating shift right narrow by immediate (second part).
    void uqshrn2(const VRegister& vd, const VRegister& vn, int shift);

    // Unsigned saturating rounding shift right narrow by immediate.
    void uqrshrn(const VRegister& vd, const VRegister& vn, int shift);

    // Unsigned saturating rounding shift right narrow by immediate (second part).
    void uqrshrn2(const VRegister& vd, const VRegister& vn, int shift);

    // Signed saturating shift right narrow by immediate.
    void sqshrn(const VRegister& vd, const VRegister& vn, int shift);

    // Signed saturating shift right narrow by immediate (second part).
    void sqshrn2(const VRegister& vd, const VRegister& vn, int shift);

    // Signed saturating rounded shift right narrow by immediate.
    void sqrshrn(const VRegister& vd, const VRegister& vn, int shift);

    // Signed saturating rounded shift right narrow by immediate (second part).
    void sqrshrn2(const VRegister& vd, const VRegister& vn, int shift);

    // Signed saturating shift right unsigned narrow by immediate.
    void sqshrun(const VRegister& vd, const VRegister& vn, int shift);

    // Signed saturating shift right unsigned narrow by immediate (second part).
    void sqshrun2(const VRegister& vd, const VRegister& vn, int shift);

    // Signed sat rounded shift right unsigned narrow by immediate.
    void sqrshrun(const VRegister& vd, const VRegister& vn, int shift);

    // Signed sat rounded shift right unsigned narrow by immediate (second part).
    void sqrshrun2(const VRegister& vd, const VRegister& vn, int shift);

    // FP reciprocal step.
    void frecps(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP reciprocal estimate.
    void frecpe(const VRegister& vd, const VRegister& vn);

    // FP reciprocal square root estimate.
    void frsqrte(const VRegister& vd, const VRegister& vn);

    // FP reciprocal square root step.
    void frsqrts(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed absolute difference and accumulate long.
    void sabal(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed absolute difference and accumulate long (second part).
    void sabal2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned absolute difference and accumulate long.
    void uabal(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned absolute difference and accumulate long (second part).
    void uabal2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed absolute difference long.
    void sabdl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed absolute difference long (second part).
    void sabdl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned absolute difference long.
    void uabdl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned absolute difference long (second part).
    void uabdl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Polynomial multiply long.
    void pmull(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Polynomial multiply long (second part).
    void pmull2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed long multiply-add.
    void smlal(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed long multiply-add (second part).
    void smlal2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned long multiply-add.
    void umlal(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned long multiply-add (second part).
    void umlal2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed long multiply-sub.
    void smlsl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed long multiply-sub (second part).
    void smlsl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned long multiply-sub.
    void umlsl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned long multiply-sub (second part).
    void umlsl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed long multiply.
    void smull(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed long multiply (second part).
    void smull2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed saturating doubling long multiply-add.
    void sqdmlal(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed saturating doubling long multiply-add (second part).
    void sqdmlal2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned absolute difference.
    void uabd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed absolute difference and accumulate.
    void saba(const VRegister& vd, const VRegister& vn, const VRegister& vm);
    
    // FP instructions.
    // Move immediate to FP register.
    void fmov(const VRegister& fd, double imm);
    void fmov(const VRegister& fd, float imm);

    // Move FP register to register.
    void fmov(const Register& rd, const VRegister& fn);

    // Move register to FP register.
    void fmov(const VRegister& fd, const Register& rn);

    // Move FP register to FP register.
    void fmov(const VRegister& fd, const VRegister& fn);

    // Move 64-bit register to top half of 128-bit FP register.
    void fmov(const VRegister& vd, int index, const Register& rn);

    // Move top half of 128-bit FP register to 64-bit register.
    void fmov(const Register& rd, const VRegister& vn, int index);

    // FP add.
    void fadd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP subtract.
    void fsub(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP multiply.
    void fmul(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP compare equal to zero.
    void fcmeq(const VRegister& vd, const VRegister& vn, double imm);

    // FP greater than zero.
    void fcmgt(const VRegister& vd, const VRegister& vn, double imm);

    // FP greater than or equal to zero.
    void fcmge(const VRegister& vd, const VRegister& vn, double imm);

    // FP less than or equal to zero.
    void fcmle(const VRegister& vd, const VRegister& vn, double imm);

    // FP less than to zero.
    void fcmlt(const VRegister& vd, const VRegister& vn, double imm);

    // FP absolute difference.
    void fabd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP pairwise add vector.
    void faddp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP pairwise add scalar.
    void faddp(const VRegister& vd, const VRegister& vn);

    // FP pairwise maximum scalar.
    void fmaxp(const VRegister& vd, const VRegister& vn);

    // FP pairwise maximum number scalar.
    void fmaxnmp(const VRegister& vd, const VRegister& vn);

    // FP pairwise minimum number scalar.
    void fminnmp(const VRegister& vd, const VRegister& vn);

    // FP vector multiply accumulate.
    void fmla(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP vector multiply subtract.
    void fmls(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP vector multiply extended.
    void fmulx(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP absolute greater than or equal.
    void facge(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP absolute greater than.
    void facgt(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP multiply by element.
    void fmul(const VRegister& vd, const VRegister& vn, const VRegister& vm,
              int vm_index);

    // FP fused multiply-add to accumulator by element.
    void fmla(const VRegister& vd, const VRegister& vn, const VRegister& vm,
              int vm_index);

    // FP fused multiply-sub from accumulator by element.
    void fmls(const VRegister& vd, const VRegister& vn, const VRegister& vm,
              int vm_index);

    // FP multiply extended by element.
    void fmulx(const VRegister& vd, const VRegister& vn, const VRegister& vm,
               int vm_index);

    // FP compare equal.
    void fcmeq(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP greater than.
    void fcmgt(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP greater than or equal.
    void fcmge(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP pairwise maximum vector.
    void fmaxp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP pairwise minimum vector.
    void fminp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP pairwise minimum scalar.
    void fminp(const VRegister& vd, const VRegister& vn);

    // FP pairwise maximum number vector.
    void fmaxnmp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP pairwise minimum number vector.
    void fminnmp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP fused multiply-add.
    void fmadd(const VRegister& vd, const VRegister& vn, const VRegister& vm,
               const VRegister& va);

    // FP fused multiply-subtract.
    void fmsub(const VRegister& vd, const VRegister& vn, const VRegister& vm,
               const VRegister& va);

    // FP fused multiply-add and negate.
    void fnmadd(const VRegister& vd, const VRegister& vn, const VRegister& vm,
                const VRegister& va);

    // FP fused multiply-subtract and negate.
    void fnmsub(const VRegister& vd, const VRegister& vn, const VRegister& vm,
                const VRegister& va);

    // FP multiply-negate scalar.
    void fnmul(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP reciprocal exponent scalar.
    void frecpx(const VRegister& vd, const VRegister& vn);

    // FP divide.
    void fdiv(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP maximum.
    void fmax(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP minimum.
    void fmin(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP maximum.
    void fmaxnm(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP minimum.
    void fminnm(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // FP absolute.
    void fabs(const VRegister& vd, const VRegister& vn);

    // FP negate.
    void fneg(const VRegister& vd, const VRegister& vn);

    // FP square root.
    void fsqrt(const VRegister& vd, const VRegister& vn);

    // FP round to integer nearest with ties to away.
    void frinta(const VRegister& vd, const VRegister& vn);

    // FP round to integer, implicit rounding.
    void frinti(const VRegister& vd, const VRegister& vn);

    // FP round to integer toward minus infinity.
    void frintm(const VRegister& vd, const VRegister& vn);

    // FP round to integer nearest with ties to even.
    void frintn(const VRegister& vd, const VRegister& vn);

    // FP round to integer towards plus infinity.
    void frintp(const VRegister& vd, const VRegister& vn);

    // FP round to integer, exact, implicit rounding.
    void frintx(const VRegister& vd, const VRegister& vn);

    // FP round to integer towards zero.
    void frintz(const VRegister& vd, const VRegister& vn);

    // FP compare registers.
    void fcmp(const VRegister& vn, const VRegister& vm);

    // FP compare immediate.
    void fcmp(const VRegister& vn, double value);

    // FP conditional compare.
    void fccmp(const VRegister& vn, const VRegister& vm, StatusFlags nzcv,
               Condition cond);

    // FP conditional select.
    void fcsel(const VRegister& vd, const VRegister& vn, const VRegister& vm,
               Condition cond);

    // Common FP Convert functions.
//    void NEONFPConvertToInt(const Register& rd, const VRegister& vn, Instr op);
//    void NEONFPConvertToInt(const VRegister& vd, const VRegister& vn, Instr op);

    // FP convert between precisions.
    void fcvt(const VRegister& vd, const VRegister& vn);

    // FP convert to higher precision.
    void fcvtl(const VRegister& vd, const VRegister& vn);

    // FP convert to higher precision (second part).
    void fcvtl2(const VRegister& vd, const VRegister& vn);

    // FP convert to lower precision.
    void fcvtn(const VRegister& vd, const VRegister& vn);

    // FP convert to lower prevision (second part).
    void fcvtn2(const VRegister& vd, const VRegister& vn);

    // FP convert to lower precision, rounding to odd.
    void fcvtxn(const VRegister& vd, const VRegister& vn);

    // FP convert to lower precision, rounding to odd (second part).
    void fcvtxn2(const VRegister& vd, const VRegister& vn);

    // FP convert to signed integer, nearest with ties to away.
    void fcvtas(const Register& rd, const VRegister& vn);

    // FP convert to unsigned integer, nearest with ties to away.
    void fcvtau(const Register& rd, const VRegister& vn);

    // FP convert to signed integer, nearest with ties to away.
    void fcvtas(const VRegister& vd, const VRegister& vn);

    // FP convert to unsigned integer, nearest with ties to away.
    void fcvtau(const VRegister& vd, const VRegister& vn);

    // FP convert to signed integer, round towards -infinity.
    void fcvtms(const Register& rd, const VRegister& vn);

    // FP convert to unsigned integer, round towards -infinity.
    void fcvtmu(const Register& rd, const VRegister& vn);

    // FP convert to signed integer, round towards -infinity.
    void fcvtms(const VRegister& vd, const VRegister& vn);

    // FP convert to unsigned integer, round towards -infinity.
    void fcvtmu(const VRegister& vd, const VRegister& vn);

    // FP convert to signed integer, nearest with ties to even.
    void fcvtns(const Register& rd, const VRegister& vn);

    // FP JavaScript convert to signed integer, rounding toward zero [Armv8.3].
    void fjcvtzs(const Register& rd, const VRegister& vn);

    // FP convert to unsigned integer, nearest with ties to even.
    void fcvtnu(const Register& rd, const VRegister& vn);

    // FP convert to signed integer, nearest with ties to even.
    void fcvtns(const VRegister& rd, const VRegister& vn);

    // FP convert to unsigned integer, nearest with ties to even.
    void fcvtnu(const VRegister& rd, const VRegister& vn);

    // FP convert to signed integer or fixed-point, round towards zero.
    void fcvtzs(const Register& rd, const VRegister& vn, int fbits = 0);

    // FP convert to unsigned integer or fixed-point, round towards zero.
    void fcvtzu(const Register& rd, const VRegister& vn, int fbits = 0);

    // FP convert to signed integer or fixed-point, round towards zero.
    void fcvtzs(const VRegister& vd, const VRegister& vn, int fbits = 0);

    // FP convert to unsigned integer or fixed-point, round towards zero.
    void fcvtzu(const VRegister& vd, const VRegister& vn, int fbits = 0);

    // FP convert to signed integer, round towards +infinity.
    void fcvtps(const Register& rd, const VRegister& vn);

    // FP convert to unsigned integer, round towards +infinity.
    void fcvtpu(const Register& rd, const VRegister& vn);

    // FP convert to signed integer, round towards +infinity.
    void fcvtps(const VRegister& vd, const VRegister& vn);

    // FP convert to unsigned integer, round towards +infinity.
    void fcvtpu(const VRegister& vd, const VRegister& vn);

    // Convert signed integer or fixed point to FP.
    void scvtf(const VRegister& fd, const Register& rn, int fbits = 0);

    // Convert unsigned integer or fixed point to FP.
    void ucvtf(const VRegister& fd, const Register& rn, int fbits = 0);

    // Convert signed integer or fixed-point to FP.
    void scvtf(const VRegister& fd, const VRegister& vn, int fbits = 0);

    // Convert unsigned integer or fixed-point to FP.
    void ucvtf(const VRegister& fd, const VRegister& vn, int fbits = 0);

    // Extract vector from pair of vectors.
    void ext(const VRegister& vd, const VRegister& vn, const VRegister& vm,
             int index);

    // Duplicate vector element to vector or scalar.
    void dup(const VRegister& vd, const VRegister& vn, int vn_index);

    // Duplicate general-purpose register to vector.
    void dup(const VRegister& vd, const Register& rn);

    // Insert vector element from general-purpose register.
    void ins(const VRegister& vd, int vd_index, const Register& rn);

    // Move general-purpose register to a vector element.
    void mov(const VRegister& vd, int vd_index, const Register& rn);

    // Unsigned move vector element to general-purpose register.
    void umov(const Register& rd, const VRegister& vn, int vn_index);

    // Move vector element to general-purpose register.
    void mov(const Register& rd, const VRegister& vn, int vn_index);

    // Move vector element to scalar.
    void mov(const VRegister& vd, const VRegister& vn, int vn_index);

    // Insert vector element from another vector element.
    void ins(const VRegister& vd, int vd_index, const VRegister& vn,
             int vn_index);

    // Move vector element to another vector element.
    void mov(const VRegister& vd, int vd_index, const VRegister& vn,
             int vn_index);

    // Signed move vector element to general-purpose register.
    void smov(const Register& rd, const VRegister& vn, int vn_index);

    // One-element structure load to one register.
    void ld1(const VRegister& vt, const MemOperand& src);

    // One-element structure load to two registers.
    void ld1(const VRegister& vt, const VRegister& vt2, const MemOperand& src);

    // One-element structure load to three registers.
    void ld1(const VRegister& vt, const VRegister& vt2, const VRegister& vt3,
             const MemOperand& src);

    // One-element structure load to four registers.
    void ld1(const VRegister& vt, const VRegister& vt2, const VRegister& vt3,
             const VRegister& vt4, const MemOperand& src);

    // One-element single structure load to one lane.
    void ld1(const VRegister& vt, int lane, const MemOperand& src);

    // One-element single structure load to all lanes.
    void ld1r(const VRegister& vt, const MemOperand& src);

    // Two-element structure load.
    void ld2(const VRegister& vt, const VRegister& vt2, const MemOperand& src);

    // Two-element single structure load to one lane.
    void ld2(const VRegister& vt, const VRegister& vt2, int lane,
             const MemOperand& src);

    // Two-element single structure load to all lanes.
    void ld2r(const VRegister& vt, const VRegister& vt2, const MemOperand& src);

    // Three-element structure load.
    void ld3(const VRegister& vt, const VRegister& vt2, const VRegister& vt3,
             const MemOperand& src);

    // Three-element single structure load to one lane.
    void ld3(const VRegister& vt, const VRegister& vt2, const VRegister& vt3,
             int lane, const MemOperand& src);

    // Three-element single structure load to all lanes.
    void ld3r(const VRegister& vt, const VRegister& vt2, const VRegister& vt3,
              const MemOperand& src);

    // Four-element structure load.
    void ld4(const VRegister& vt, const VRegister& vt2, const VRegister& vt3,
             const VRegister& vt4, const MemOperand& src);

    // Four-element single structure load to one lane.
    void ld4(const VRegister& vt, const VRegister& vt2, const VRegister& vt3,
             const VRegister& vt4, int lane, const MemOperand& src);

    // Four-element single structure load to all lanes.
    void ld4r(const VRegister& vt, const VRegister& vt2, const VRegister& vt3,
              const VRegister& vt4, const MemOperand& src);

    // Count leading sign bits.
    void cls(const VRegister& vd, const VRegister& vn);

    // Count leading zero bits (vector).
    void clz(const VRegister& vd, const VRegister& vn);

    // Population count per byte.
    void cnt(const VRegister& vd, const VRegister& vn);

    // Reverse bit order.
    void rbit(const VRegister& vd, const VRegister& vn);

    // Reverse elements in 16-bit halfwords.
    void rev16(const VRegister& vd, const VRegister& vn);

    // Reverse elements in 32-bit words.
    void rev32(const VRegister& vd, const VRegister& vn);

    // Reverse elements in 64-bit doublewords.
    void rev64(const VRegister& vd, const VRegister& vn);

    // Unsigned reciprocal square root estimate.
    void ursqrte(const VRegister& vd, const VRegister& vn);

    // Unsigned reciprocal estimate.
    void urecpe(const VRegister& vd, const VRegister& vn);

    // Signed pairwise long add and accumulate.
    void sadalp(const VRegister& vd, const VRegister& vn);

    // Signed pairwise long add.
    void saddlp(const VRegister& vd, const VRegister& vn);

    // Unsigned pairwise long add.
    void uaddlp(const VRegister& vd, const VRegister& vn);

    // Unsigned pairwise long add and accumulate.
    void uadalp(const VRegister& vd, const VRegister& vn);

    // Shift left by immediate.
    void shl(const VRegister& vd, const VRegister& vn, int shift);

    // Signed saturating shift left by immediate.
    void sqshl(const VRegister& vd, const VRegister& vn, int shift);

    // Signed saturating shift left unsigned by immediate.
    void sqshlu(const VRegister& vd, const VRegister& vn, int shift);

    // Unsigned saturating shift left by immediate.
    void uqshl(const VRegister& vd, const VRegister& vn, int shift);

    // Signed shift left long by immediate.
    void sshll(const VRegister& vd, const VRegister& vn, int shift);

    // Signed shift left long by immediate (second part).
    void sshll2(const VRegister& vd, const VRegister& vn, int shift);

    // Signed extend long.
    void sxtl(const VRegister& vd, const VRegister& vn);

    // Signed extend long (second part).
    void sxtl2(const VRegister& vd, const VRegister& vn);

    // Unsigned shift left long by immediate.
    void ushll(const VRegister& vd, const VRegister& vn, int shift);

    // Unsigned shift left long by immediate (second part).
    void ushll2(const VRegister& vd, const VRegister& vn, int shift);

    // Shift left long by element size.
    void shll(const VRegister& vd, const VRegister& vn, int shift);

    // Shift left long by element size (second part).
    void shll2(const VRegister& vd, const VRegister& vn, int shift);

    // Unsigned extend long.
    void uxtl(const VRegister& vd, const VRegister& vn);

    // Unsigned extend long (second part).
    void uxtl2(const VRegister& vd, const VRegister& vn);

    // Signed rounding halving add.
    void srhadd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned halving sub.
    void uhsub(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed halving sub.
    void shsub(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned saturating add.
    void uqadd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed saturating add.
    void sqadd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Unsigned saturating subtract.
    void uqsub(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Signed saturating subtract.
    void sqsub(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Add pairwise.
    void addp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Add pair of elements scalar.
    void addp(const VRegister& vd, const VRegister& vn);

    // Multiply-add to accumulator.
    void mla(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Multiply-subtract to accumulator.
    void mls(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Multiply.
    void mul(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Table lookup from one register.
    void tbl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Table lookup from two registers.
    void tbl(const VRegister& vd, const VRegister& vn, const VRegister& vn2,
             const VRegister& vm);

    // Table lookup from three registers.
    void tbl(const VRegister& vd, const VRegister& vn, const VRegister& vn2,
             const VRegister& vn3, const VRegister& vm);

    // Table lookup from four registers.
    void tbl(const VRegister& vd, const VRegister& vn, const VRegister& vn2,
             const VRegister& vn3, const VRegister& vn4, const VRegister& vm);

    // Table lookup extension from one register.
    void tbx(const VRegister& vd, const VRegister& vn, const VRegister& vm);

    // Table lookup extension from two registers.
    void tbx(const VRegister& vd, const VRegister& vn, const VRegister& vn2,
             const VRegister& vm);

    // Table lookup extension from three registers.
    void tbx(const VRegister& vd, const VRegister& vn, const VRegister& vn2,
             const VRegister& vn3, const VRegister& vm);

    // Table lookup extension from four registers.
    void tbx(const VRegister& vd, const VRegister& vn, const VRegister& vn2,
             const VRegister& vn3, const VRegister& vn4, const VRegister& vm);

    // Instruction functions used only for test, debug, and patching.
    // Emit raw instructions in the instruction stream.
    void dci(uint32_t raw_inst) { Emit(raw_inst); }

    // Emit 8 bits of data in the instruction stream.
    void dc8(uint8_t data) { EmitData(&data, sizeof(data)); }

    // Emit 32 bits of data in the instruction stream.
    void dc32(uint32_t data) { EmitData(&data, sizeof(data)); }

    // Emit 64 bits of data in the instruction stream.
    void dc64(uint64_t data) { EmitData(&data, sizeof(data)); }

    // Emit an address in the instruction stream.
    void dcptr(Label* label);
    // End of Instruction set functions --------------------------------------------------------------------------------
    
private:
    void ConditionalSelect(const Register& rd, const Register& rn, const Register& rm, Condition cond,
                           ConditionalSelectOp op) {
        assert(rd.size_in_bits() == rn.size_in_bits());
        assert(rd.size_in_bits() == rm.size_in_bits());
        Emit(SF(rd) | op | Rm(rm) | Cond(cond) | Rn(rn) | Rd(rd));
    }
    
    void ConditionalCompare(const Register& rn, const Operand& operand, StatusFlags nzcv, Condition cond,
                            ConditionalCompareOp op) {
        uint32_t ccmpop;
        //assert(!operand.NeedsRelocation(this));
        if (operand.IsImmediate()) {
            int64_t immediate = operand.immediate_value();
            assert(IsImmConditionalCompare(immediate));
            ccmpop = ConditionalCompareImmediateFixed | op |
                 ImmCondCmp(static_cast<unsigned>(immediate));
        } else {
            assert(operand.IsShiftedRegister() && (operand.shift_amount() == 0));
            ccmpop = ConditionalCompareRegisterFixed | op | Rm(operand.reg());
        }
        Emit(SF(rn) | ccmpop | Cond(cond) | Rn(rn) | Nzcv(nzcv));
    }
    
    static bool IsImmConditionalCompare(int64_t immediate) { return base::is_uint5(immediate); }
    
    void DataProcessing3Source(const Register& rd, const Register& rn, const Register& rm, const Register& ra,
                               DataProcessing3SourceOp op) {
        Emit(SF(rd) | op | Rm(rm) | Ra(ra) | Rn(rn) | Rd(rd));
    }
    
    void DataProcessing1Source(const Register& rd, const Register& rn, DataProcessing1SourceOp op) {
        assert(rd.size_in_bits() == rn.size_in_bits());
        Emit(SF(rn) | op | Rn(rn) | Rd(rd));
    }
    
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
    
    void LoadStore(const CPURegister& rt, const MemOperand& addr, LoadStoreOp op);
    
    unsigned CalcLSDataSize(LoadStoreOp op) {
        assert((LSSize_offset + LSSize_width) == (kInstrSize * 8));
        unsigned size = static_cast<uint32_t>(op >> LSSize_offset);
        if ((op & LSVector_mask) != 0) {
            // Vector register memory operations encode the access size in the "size"
            // and "opc" fields.
            if ((size == 0) && ((op & LSOpc_mask) >> LSOpc_offset) >= 2) {
                size = kQRegSizeLog2;
            }
        }
        return size;
    }
    
    LoadStorePairOp LoadPairOpFor(const CPURegister& rt, const CPURegister& rt2) {
      assert((STP_w | LoadStorePairLBit) == LDP_w);
      return static_cast<LoadStorePairOp>(StorePairOpFor(rt, rt2) | LoadStorePairLBit);
    }
    
    LoadStoreOp LoadOpFor(const CPURegister& rt) {
        assert(rt.is_valid());
        if (rt.IsRegister()) {
            return rt.Is64Bits() ? LDR_x : LDR_w;
        } else {
            assert(rt.IsVRegister());
            switch (rt.size_in_bits()) {
                case kBRegSizeInBits:
                    return LDR_b;
                case kHRegSizeInBits:
                    return LDR_h;
                case kSRegSizeInBits:
                    return LDR_s;
                case kDRegSizeInBits:
                    return LDR_d;
                default:
                    assert(rt.IsQ());
                    return LDR_q;
            }
        }
    }
    
    LoadStorePairOp StorePairOpFor(const CPURegister& rt, const CPURegister& rt2) {
        assert(AreSameSizeAndType(rt, rt2));
        if (rt.IsRegister()) {
            return rt.Is64Bits() ? STP_x : STP_w;
        } else {
            assert(rt.IsVRegister());
            switch (rt.size_in_bits()) {
                case kSRegSizeInBits:
                    return STP_s;
                case kDRegSizeInBits:
                    return STP_d;
                default:
                    assert(rt.IsQ());
                    return STP_q;
            }
        }
    }
    
    LoadStoreOp StoreOpFor(const CPURegister& rt) {
        assert(rt.is_valid());
        if (rt.IsRegister()) {
            return rt.Is64Bits() ? STR_x : STR_w;
        } else {
            assert(rt.IsVRegister());
            switch (rt.size_in_bits()) {
                case kBRegSizeInBits:
                    return STR_b;
                case kHRegSizeInBits:
                    return STR_h;
                case kSRegSizeInBits:
                    return STR_s;
                case kDRegSizeInBits:
                    return STR_d;
                default:
                    assert(rt.IsQ());
                    return STR_q;
            }
        }
    }

    void NEON3Same(const VRegister& vd, const VRegister& vn, const VRegister& vm, NEON3SameOp vop) {
        assert(AreSameFormat(vd, vn, vm));
        assert(vd.IsVector() || !vd.IsQ());

        uint32_t format, op = vop;
        if (vd.IsScalar()) {
            op |= NEON_Q | NEONScalar;
            format = SFormat(vd);
        } else {
            format = VFormat(vd);
        }

        Emit(format | op | Rm(vm) | Rn(vn) | Rd(vd));
    }
    
    void Logical(const Register& rd, const Register& rn, const Operand& operand, LogicalOp op);
    
    void AddSub(const Register& rd, const Register& rn, const Operand& operand, FlagsUpdate S, AddSubOp op);
    
    void AddSubWithCarry(const Register& rd, const Register& rn, const Operand& operand, FlagsUpdate S,
                         AddSubWithCarryOp op) {
        assert(rd.size_in_bits() == rn.size_in_bits());
        assert(rd.size_in_bits() == operand.reg().size_in_bits());
        assert(operand.IsShiftedRegister() && (operand.shift_amount() == 0));
        //assert(!operand.NeedsRelocation(this));
        Emit(SF(rd) | op | Flags(S) | Rm(operand.reg()) | Rn(rn) | Rd(rd));
    }
    
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
    
    static uint32_t ImmLSUnsigned(int imm12) {
        assert(base::is_uint12(imm12));
        return imm12 << ImmLSUnsigned_offset;
    }

    static uint32_t ImmLS(int imm9) {
        assert(base::is_int9(imm9));
        return base::truncate_to_int9(imm9) << ImmLS_offset;
    }

    static uint32_t ImmLSPair(int imm7, unsigned size) {
        assert(imm7 == static_cast<int>(static_cast<uint32_t>(imm7 >> size) << size));
        int scaled_imm7 = imm7 >> size;
        assert(base::is_int7(scaled_imm7));
        return base::truncate_to_int7(scaled_imm7) << ImmLSPair_offset;
    }

    static uint32_t ImmShiftLS(unsigned shift_amount) {
        assert(base::is_uint1(shift_amount));
        return shift_amount << ImmShiftLS_offset;
    }

    static uint32_t ImmException(int imm16) {
        assert(base::is_uint16(imm16));
        return imm16 << ImmException_offset;
    }

    static uint32_t ImmSystemRegister(int imm15) {
        assert(base::is_uint15(imm15));
        return imm15 << ImmSystemRegister_offset;
    }

    static uint32_t ImmHint(int imm7) {
        assert(base::is_uint7(imm7));
        return imm7 << ImmHint_offset;
    }

    static uint32_t ImmBarrierDomain(int imm2) {
        assert(base::is_uint2(imm2));
        return imm2 << ImmBarrierDomain_offset;
    }

    static uint32_t ImmBarrierType(int imm2) {
        assert(base::is_uint2(imm2));
        return imm2 << ImmBarrierType_offset;
    }

    static uint32_t ImmMoveWide(int imm) {
        assert(base::is_uint16(imm));
        return imm << ImmMoveWide_offset;
    }

    static uint32_t ShiftMoveWide(int shift) {
        assert(base::is_uint2(shift));
        return shift << ShiftMoveWide_offset;
    }
    
    static bool IsImmLSUnscaled(int64_t offset) { return base::is_int9(offset); }
    static bool IsImmLSScaled(int64_t offset, unsigned size) {
        bool offset_is_size_multiple = (static_cast<int64_t>(static_cast<uint64_t>(offset >> size) << size) == offset);
        return offset_is_size_multiple && base::is_uint12(offset >> size);
    }
    static bool IsImmLLiteral(int64_t offset) {
        int inst_size = static_cast<int>(kInstrSizeLog2);
        bool offset_is_inst_multiple = (static_cast<int64_t>(static_cast<uint64_t>(offset >> inst_size)
                                                             << inst_size) == offset);
        assert(offset > 0);
        offset >>= kLoadLiteralScaleLog2;
        return offset_is_inst_multiple && base::is_intn(offset, ImmLLiteral_width);
    }

    // Move immediates encoding.
    //inline static uint32_t ImmMoveWide(int imm);
    //inline static uint32_t ShiftMoveWide(int shift);

    // FP Immediates.
//    static uint32_t ImmFP(double imm);
//    static uint32_t ImmNEONFP(double imm);

    static uint32_t FPType(VRegister fd) { return fd.Is64Bits() ? FP64 : FP32; }

    static uint32_t FPScale(unsigned scale) {
        assert(base::is_uint6(scale));
        return scale << FPScale_offset;
    }

    static bool IsImmAddSub(int64_t immediate) {
        return base::is_uint12(immediate) || (base::is_uint12(immediate >> 12) && ((immediate & 0xFFF) == 0));
    }
    
    void LogicalImmediate(const Register& rd, const Register& rn, unsigned n, unsigned imm_s, unsigned imm_r,
                          LogicalOp op) {
        unsigned reg_size = rd.size_in_bits();
        uint32_t dest_reg = (op == ANDS) ? Rd(rd) : RdSP(rd);
        Emit(SF(rd) | LogicalImmediateFixed | op | BitN(n, reg_size) | ImmSetBits(imm_s, reg_size) |
             ImmRotate(imm_r, reg_size) | dest_reg | Rn(rn));
    }
    
    static bool IsImmLogical(uint64_t value, unsigned width, unsigned* n, unsigned* imm_s, unsigned* imm_r);
    
    // Instruction bits for vector format in data processing operations.
    static uint32_t VFormat(VRegister vd) {
        if (vd.Is64Bits()) {
            switch (vd.lane_count()) {
                case 2:
                    return NEON_2S;
                case 4:
                    return NEON_4H;
                case 8:
                    return NEON_8B;
                default:
                    UNREACHABLE();
            }
        } else {
            assert(vd.Is128Bits());
            switch (vd.lane_count()) {
                case 2:
                    return NEON_2D;
                case 4:
                    return NEON_4S;
                case 8:
                    return NEON_8H;
                case 16:
                    return NEON_16B;
                default:
                    UNREACHABLE();
            }
        }
    }

    // Instruction bits for scalar format in data processing operations.
    static uint32_t SFormat(VRegister vd) {
        assert(vd.IsScalar());
        switch (vd.size_in_bytes()) {
            case 1:
                return NEON_B;
            case 2:
                return NEON_H;
            case 4:
                return NEON_S;
            case 8:
                return NEON_D;
            default:
                UNREACHABLE();
        }
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

    int pc_ = 0;
    std::string buf_;
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Assembler);
}; // class Assembler


#define NEON_3SAME_LIST(V)                                         \
  V(add, NEON_ADD, vd.IsVector() || vd.Is1D())                     \
  V(addp, NEON_ADDP, vd.IsVector() || vd.Is1D())                   \
  V(sub, NEON_SUB, vd.IsVector() || vd.Is1D())                     \
  V(cmeq, NEON_CMEQ, vd.IsVector() || vd.Is1D())                   \
  V(cmge, NEON_CMGE, vd.IsVector() || vd.Is1D())                   \
  V(cmgt, NEON_CMGT, vd.IsVector() || vd.Is1D())                   \
  V(cmhi, NEON_CMHI, vd.IsVector() || vd.Is1D())                   \
  V(cmhs, NEON_CMHS, vd.IsVector() || vd.Is1D())                   \
  V(cmtst, NEON_CMTST, vd.IsVector() || vd.Is1D())                 \
  V(sshl, NEON_SSHL, vd.IsVector() || vd.Is1D())                   \
  V(ushl, NEON_USHL, vd.IsVector() || vd.Is1D())                   \
  V(srshl, NEON_SRSHL, vd.IsVector() || vd.Is1D())                 \
  V(urshl, NEON_URSHL, vd.IsVector() || vd.Is1D())                 \
  V(sqdmulh, NEON_SQDMULH, vd.IsLaneSizeH() || vd.IsLaneSizeS())   \
  V(sqrdmulh, NEON_SQRDMULH, vd.IsLaneSizeH() || vd.IsLaneSizeS()) \
  V(shadd, NEON_SHADD, vd.IsVector() && !vd.IsLaneSizeD())         \
  V(uhadd, NEON_UHADD, vd.IsVector() && !vd.IsLaneSizeD())         \
  V(srhadd, NEON_SRHADD, vd.IsVector() && !vd.IsLaneSizeD())       \
  V(urhadd, NEON_URHADD, vd.IsVector() && !vd.IsLaneSizeD())       \
  V(shsub, NEON_SHSUB, vd.IsVector() && !vd.IsLaneSizeD())         \
  V(uhsub, NEON_UHSUB, vd.IsVector() && !vd.IsLaneSizeD())         \
  V(smax, NEON_SMAX, vd.IsVector() && !vd.IsLaneSizeD())           \
  V(smaxp, NEON_SMAXP, vd.IsVector() && !vd.IsLaneSizeD())         \
  V(smin, NEON_SMIN, vd.IsVector() && !vd.IsLaneSizeD())           \
  V(sminp, NEON_SMINP, vd.IsVector() && !vd.IsLaneSizeD())         \
  V(umax, NEON_UMAX, vd.IsVector() && !vd.IsLaneSizeD())           \
  V(umaxp, NEON_UMAXP, vd.IsVector() && !vd.IsLaneSizeD())         \
  V(umin, NEON_UMIN, vd.IsVector() && !vd.IsLaneSizeD())           \
  V(uminp, NEON_UMINP, vd.IsVector() && !vd.IsLaneSizeD())         \
  V(saba, NEON_SABA, vd.IsVector() && !vd.IsLaneSizeD())           \
  V(sabd, NEON_SABD, vd.IsVector() && !vd.IsLaneSizeD())           \
  V(uaba, NEON_UABA, vd.IsVector() && !vd.IsLaneSizeD())           \
  V(uabd, NEON_UABD, vd.IsVector() && !vd.IsLaneSizeD())           \
  V(mla, NEON_MLA, vd.IsVector() && !vd.IsLaneSizeD())             \
  V(mls, NEON_MLS, vd.IsVector() && !vd.IsLaneSizeD())             \
  V(mul, NEON_MUL, vd.IsVector() && !vd.IsLaneSizeD())             \
  V(and_, NEON_AND, vd.Is8B() || vd.Is16B())                       \
  V(orr, NEON_ORR, vd.Is8B() || vd.Is16B())                        \
  V(orn, NEON_ORN, vd.Is8B() || vd.Is16B())                        \
  V(eor, NEON_EOR, vd.Is8B() || vd.Is16B())                        \
  V(bic, NEON_BIC, vd.Is8B() || vd.Is16B())                        \
  V(bit, NEON_BIT, vd.Is8B() || vd.Is16B())                        \
  V(bif, NEON_BIF, vd.Is8B() || vd.Is16B())                        \
  V(bsl, NEON_BSL, vd.Is8B() || vd.Is16B())                        \
  V(pmul, NEON_PMUL, vd.Is8B() || vd.Is16B())                      \
  V(uqadd, NEON_UQADD, true)                                       \
  V(sqadd, NEON_SQADD, true)                                       \
  V(uqsub, NEON_UQSUB, true)                                       \
  V(sqsub, NEON_SQSUB, true)                                       \
  V(sqshl, NEON_SQSHL, true)                                       \
  V(uqshl, NEON_UQSHL, true)                                       \
  V(sqrshl, NEON_SQRSHL, true)                                     \
  V(uqrshl, NEON_UQRSHL, true)

#define DEFINE_ASM_FUNC(FN, OP, AS)                                \
    inline void Assembler::FN(const VRegister& vd, const VRegister& vn, const VRegister& vm) { \
        assert(AS);                                                \
        NEON3Same(vd, vn, vm, OP);                                 \
    }
NEON_3SAME_LIST(DEFINE_ASM_FUNC)
#undef DEFINE_ASM_FUNC

} // namespace arm64

} // namespace yalx

#endif // YALX_ARM64_ASM_ARM64_H_
