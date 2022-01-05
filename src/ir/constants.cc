#include "ir/constants.h"
#include "ir/type.h"
#include "ir/metadata.h"

namespace yalx {

namespace ir {

static constexpr int kConversionRulersRefOffset = (Type::kString + 1);
static constexpr int kConversionRulersValOffset = kConversionRulersRefOffset + Model::kMaxDeclarations;
static constexpr int kConversionRulersMaxDims = (Type::kString + 1) + Model::kMaxDeclarations * 2;

static const ConversionHint kConversionRules[kConversionRulersMaxDims][kConversionRulersMaxDims] = {
    /* from [void]  */ {
    /* void         */ kKeep,
    /* byte         */ kDeny,
    /* word         */ kDeny,
    /* dword        */ kDeny,
    /* qword        */ kDeny,
    /* i8           */ kDeny,
    /* i16          */ kDeny,
    /* i32          */ kDeny,
    /* i64          */ kDeny,
    /* u8           */ kDeny,
    /* u16          */ kDeny,
    /* u32          */ kDeny,
    /* u64          */ kDeny,
    /* f32          */ kDeny,
    /* f64          */ kDeny,
    /* str          */ kDeny,
    /* ref + class  */ kDeny,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [byte]  */ {
    /* void         */ kDeny,
    /* byte         */ kKeep,
    /* word         */ kZextTo,
    /* dword        */ kZextTo,
    /* qword        */ kZextTo,
    /* i8           */ kBitCastTo,
    /* i16          */ kZextTo,
    /* i32          */ kZextTo,
    /* i64          */ kZextTo,
    /* u8           */ kBitCastTo,
    /* u16          */ kZextTo,
    /* u32          */ kZextTo,
    /* u64          */ kZextTo,
    /* f32          */ kUIToFP,
    /* f64          */ kUIToFP,
    /* str          */ kDeny,
    /* ref + class  */ kBoxingTo,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [word]  */ {
    /* void         */ kDeny,
    /* byte         */ kTruncTo,
    /* word         */ kKeep,
    /* dword        */ kZextTo,
    /* qword        */ kZextTo,
    /* i8           */ kTruncTo,
    /* i16          */ kBitCastTo,
    /* i32          */ kZextTo,
    /* i64          */ kZextTo,
    /* u8           */ kTruncTo,
    /* u16          */ kBitCastTo,
    /* u32          */ kZextTo,
    /* u64          */ kZextTo,
    /* f32          */ kUIToFP,
    /* f64          */ kUIToFP,
    /* str          */ kDeny,
    /* ref + class  */ kBoxingTo,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [dword] */ {
    /* void         */ kDeny,
    /* byte         */ kTruncTo,
    /* word         */ kTruncTo,
    /* dword        */ kKeep,
    /* qword        */ kZextTo,
    /* i8           */ kTruncTo,
    /* i16          */ kTruncTo,
    /* i32          */ kBitCastTo,
    /* i64          */ kZextTo,
    /* u8           */ kTruncTo,
    /* u16          */ kTruncTo,
    /* u32          */ kBitCastTo,
    /* u64          */ kZextTo,
    /* f32          */ kUIToFP,
    /* f64          */ kUIToFP,
    /* str          */ kDeny,
    /* ref + class  */ kBoxingTo,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [qword] */ {
    /* void         */ kDeny,
    /* byte         */ kTruncTo,
    /* word         */ kTruncTo,
    /* dword        */ kTruncTo,
    /* qword        */ kKeep,
    /* i8           */ kTruncTo,
    /* i16          */ kTruncTo,
    /* i32          */ kTruncTo,
    /* i64          */ kBitCastTo,
    /* u8           */ kTruncTo,
    /* u16          */ kTruncTo,
    /* u32          */ kTruncTo,
    /* u64          */ kBitCastTo,
    /* f32          */ kUIToFP,
    /* f64          */ kUIToFP,
    /* str          */ kDeny,
    /* ref + class  */ kBoxingTo,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [i8]    */ {
    /* void         */ kDeny,
    /* byte         */ kBitCastTo,
    /* word         */ kZextTo,
    /* dword        */ kZextTo,
    /* qword        */ kZextTo,
    /* i8           */ kKeep,
    /* i16          */ kSextTo,
    /* i32          */ kSextTo,
    /* i64          */ kSextTo,
    /* u8           */ kBitCastTo,
    /* u16          */ kZextTo,
    /* u32          */ kZextTo,
    /* u64          */ kZextTo,
    /* f32          */ kSIToFP,
    /* f64          */ kSIToFP,
    /* str          */ kDeny,
    /* ref + class  */ kBoxingTo,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [i16]   */ {
    /* void         */ kDeny,
    /* byte         */ kTruncTo,
    /* word         */ kBitCastTo,
    /* dword        */ kZextTo,
    /* qword        */ kZextTo,
    /* i8           */ kTruncTo,
    /* i16          */ kKeep,
    /* i32          */ kSextTo,
    /* i64          */ kSextTo,
    /* u8           */ kTruncTo,
    /* u16          */ kBitCastTo,
    /* u32          */ kZextTo,
    /* u64          */ kZextTo,
    /* f32          */ kSIToFP,
    /* f64          */ kSIToFP,
    /* str          */ kDeny,
    /* ref + class  */ kBoxingTo,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [i32]   */ {
    /* void         */ kDeny,
    /* byte         */ kTruncTo,
    /* word         */ kTruncTo,
    /* dword        */ kBitCastTo,
    /* qword        */ kZextTo,
    /* i8           */ kTruncTo,
    /* i16          */ kTruncTo,
    /* i32          */ kKeep,
    /* i64          */ kSextTo,
    /* u8           */ kTruncTo,
    /* u16          */ kTruncTo,
    /* u32          */ kBitCastTo,
    /* u64          */ kZextTo,
    /* f32          */ kSIToFP,
    /* f64          */ kSIToFP,
    /* str          */ kDeny,
    /* ref + class  */ kBoxingTo,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [i64]   */ {
    /* void         */ kDeny,
    /* byte         */ kTruncTo,
    /* word         */ kTruncTo,
    /* dword        */ kTruncTo,
    /* qword        */ kBitCastTo,
    /* i8           */ kTruncTo,
    /* i16          */ kTruncTo,
    /* i32          */ kTruncTo,
    /* i64          */ kKeep,
    /* u8           */ kTruncTo,
    /* u16          */ kTruncTo,
    /* u32          */ kTruncTo,
    /* u64          */ kBitCastTo,
    /* f32          */ kSIToFP,
    /* f64          */ kSIToFP,
    /* str          */ kDeny,
    /* ref + class  */ kBoxingTo,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [u8]    */ {
    /* void         */ kDeny,
    /* byte         */ kBitCastTo,
    /* word         */ kZextTo,
    /* dword        */ kZextTo,
    /* qword        */ kZextTo,
    /* i8           */ kBitCastTo,
    /* i16          */ kZextTo,
    /* i32          */ kZextTo,
    /* i64          */ kZextTo,
    /* u8           */ kKeep,
    /* u16          */ kZextTo,
    /* u32          */ kZextTo,
    /* u64          */ kZextTo,
    /* f32          */ kUIToFP,
    /* f64          */ kUIToFP,
    /* str          */ kDeny,
    /* ref + class  */ kBoxingTo,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [u16]   */ {
    /* void         */ kDeny,
    /* byte         */ kTruncTo,
    /* word         */ kBitCastTo,
    /* dword        */ kZextTo,
    /* qword        */ kZextTo,
    /* i8           */ kTruncTo,
    /* i16          */ kBitCastTo,
    /* i32          */ kZextTo,
    /* i64          */ kZextTo,
    /* u8           */ kTruncTo,
    /* u16          */ kKeep,
    /* u32          */ kZextTo,
    /* u64          */ kZextTo,
    /* f32          */ kUIToFP,
    /* f64          */ kUIToFP,
    /* str          */ kDeny,
    /* ref + class  */ kBoxingTo,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [u32]   */ {
    /* void         */ kDeny,
    /* byte         */ kTruncTo,
    /* word         */ kTruncTo,
    /* dword        */ kBitCastTo,
    /* qword        */ kZextTo,
    /* i8           */ kTruncTo,
    /* i16          */ kTruncTo,
    /* i32          */ kBitCastTo,
    /* i64          */ kZextTo,
    /* u8           */ kTruncTo,
    /* u16          */ kTruncTo,
    /* u32          */ kKeep,
    /* u64          */ kZextTo,
    /* f32          */ kUIToFP,
    /* f64          */ kUIToFP,
    /* str          */ kDeny,
    /* ref + class  */ kBoxingTo,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [u64]   */ {
    /* void         */ kDeny,
    /* byte         */ kTruncTo,
    /* word         */ kTruncTo,
    /* dword        */ kTruncTo,
    /* qword        */ kBitCastTo,
    /* i8           */ kTruncTo,
    /* i16          */ kTruncTo,
    /* i32          */ kTruncTo,
    /* i64          */ kBitCastTo,
    /* u8           */ kTruncTo,
    /* u16          */ kTruncTo,
    /* u32          */ kTruncTo,
    /* u64          */ kKeep,
    /* f32          */ kUIToFP,
    /* f64          */ kUIToFP,
    /* str          */ kDeny,
    /* ref + class  */ kBoxingTo,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [f32]   */ {
    /* void         */ kDeny,
    /* byte         */ kFPToUI,
    /* word         */ kFPToUI,
    /* dword        */ kFPToUI,
    /* qword        */ kFPToUI,
    /* i8           */ kFPToSI,
    /* i16          */ kFPToSI,
    /* i32          */ kFPToSI,
    /* i64          */ kFPToSI,
    /* u8           */ kFPToUI,
    /* u16          */ kFPToUI,
    /* u32          */ kFPToUI,
    /* u64          */ kFPToUI,
    /* f32          */ kKeep,
    /* f64          */ kFPExtTo,
    /* str          */ kDeny,
    /* ref + class  */ kBoxingTo,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [f64]   */ {
    /* void         */ kDeny,
    /* byte         */ kFPToUI,
    /* word         */ kFPToUI,
    /* dword        */ kFPToUI,
    /* qword        */ kFPToUI,
    /* i8           */ kFPToSI,
    /* i16          */ kFPToSI,
    /* i32          */ kFPToSI,
    /* i64          */ kFPToSI,
    /* u8           */ kFPToUI,
    /* u16          */ kFPToUI,
    /* u32          */ kFPToUI,
    /* u64          */ kFPToUI,
    /* f32          */ kFPTruncTo,
    /* f64          */ kKeep,
    /* str          */ kDeny,
    /* ref + class  */ kBoxingTo,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [str]   */ {
    /* void         */ kDeny,
    /* byte         */ kDeny,
    /* word         */ kDeny,
    /* dword        */ kDeny,
    /* qword        */ kDeny,
    /* i8           */ kDeny,
    /* i16          */ kDeny,
    /* i32          */ kDeny,
    /* i64          */ kDeny,
    /* u8           */ kDeny,
    /* u16          */ kDeny,
    /* u32          */ kDeny,
    /* u64          */ kDeny,
    /* f32          */ kDeny,
    /* f64          */ kDeny,
    /* str          */ kKeep,
    /* ref + class  */ kKeep,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [ref + class] */ {
    /* void         */ kKeep,
    /* byte         */ kUnboxingTo,
    /* word         */ kUnboxingTo,
    /* dword        */ kUnboxingTo,
    /* qword        */ kUnboxingTo,
    /* i8           */ kUnboxingTo,
    /* i16          */ kUnboxingTo,
    /* i32          */ kUnboxingTo,
    /* i64          */ kUnboxingTo,
    /* u8           */ kUnboxingTo,
    /* u16          */ kUnboxingTo,
    /* u32          */ kUnboxingTo,
    /* u64          */ kUnboxingTo,
    /* f32          */ kUnboxingTo,
    /* f64          */ kUnboxingTo,
    /* str          */ kRefAssertedTo,
    /* ref + class  */ kRefAssertedTo,
    /* ref + struct */ kRefAssertedTo,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kRefAssertedTo,
    /* ref + array  */ kRefAssertedTo,
    /* ref + chan   */ kRefAssertedTo,
    /* val + class  */ kUnboxingTo,
    /* val + struct */ kUnboxingTo,
    /* val + iface  */ kRefToIface,
    /* val + fun    */ kUnboxingTo,
    /* val + array  */ kUnboxingTo,
    /* val + chan   */ kUnboxingTo,
    },
    /* from [ref + struct] */ {
    /* void         */ kKeep,
    /* byte         */ kUnboxingTo,
    /* word         */ kUnboxingTo,
    /* dword        */ kUnboxingTo,
    /* qword        */ kUnboxingTo,
    /* i8           */ kUnboxingTo,
    /* i16          */ kUnboxingTo,
    /* i32          */ kUnboxingTo,
    /* i64          */ kUnboxingTo,
    /* u8           */ kUnboxingTo,
    /* u16          */ kUnboxingTo,
    /* u32          */ kUnboxingTo,
    /* u64          */ kUnboxingTo,
    /* f32          */ kUnboxingTo,
    /* f64          */ kUnboxingTo,
    /* str          */ kRefAssertedTo,
    /* ref + class  */ kDeny,
    /* ref + struct */ kBoxingTo,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kUnboxingTo,
    /* val + iface  */ kUnboxingTo,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [ref + iface] */ {
    /* void         */ kDeny,
    /* byte         */ kDeny,
    /* word         */ kDeny,
    /* dword        */ kDeny,
    /* qword        */ kDeny,
    /* i8           */ kDeny,
    /* i16          */ kDeny,
    /* i32          */ kDeny,
    /* i64          */ kDeny,
    /* u8           */ kDeny,
    /* u16          */ kDeny,
    /* u32          */ kDeny,
    /* u64          */ kDeny,
    /* f32          */ kDeny,
    /* f64          */ kDeny,
    /* str          */ kDeny,
    /* ref + class  */ kDeny,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [ref + fun] */ {
    /* void         */ kDeny,
    /* byte         */ kDeny,
    /* word         */ kDeny,
    /* dword        */ kDeny,
    /* qword        */ kDeny,
    /* i8           */ kDeny,
    /* i16          */ kDeny,
    /* i32          */ kDeny,
    /* i64          */ kDeny,
    /* u8           */ kDeny,
    /* u16          */ kDeny,
    /* u32          */ kDeny,
    /* u64          */ kDeny,
    /* f32          */ kDeny,
    /* f64          */ kDeny,
    /* str          */ kDeny,
    /* ref + class  */ kKeep,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kKeep,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [ref + array] */ {
    /* void         */ kDeny,
    /* byte         */ kDeny,
    /* word         */ kDeny,
    /* dword        */ kDeny,
    /* qword        */ kDeny,
    /* i8           */ kDeny,
    /* i16          */ kDeny,
    /* i32          */ kDeny,
    /* i64          */ kDeny,
    /* u8           */ kDeny,
    /* u16          */ kDeny,
    /* u32          */ kDeny,
    /* u64          */ kDeny,
    /* f32          */ kDeny,
    /* f64          */ kDeny,
    /* str          */ kDeny,
    /* ref + class  */ kKeep,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kKeep,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [ref + chan] */ {
    /* void         */ kDeny,
    /* byte         */ kDeny,
    /* word         */ kDeny,
    /* dword        */ kDeny,
    /* qword        */ kDeny,
    /* i8           */ kDeny,
    /* i16          */ kDeny,
    /* i32          */ kDeny,
    /* i64          */ kDeny,
    /* u8           */ kDeny,
    /* u16          */ kDeny,
    /* u32          */ kDeny,
    /* u64          */ kDeny,
    /* f32          */ kDeny,
    /* f64          */ kDeny,
    /* str          */ kDeny,
    /* ref + class  */ kKeep,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kKeep,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [val + class] */ {
    /* void         */ kDeny,
    /* byte         */ kDeny,
    /* word         */ kDeny,
    /* dword        */ kDeny,
    /* qword        */ kDeny,
    /* i8           */ kDeny,
    /* i16          */ kDeny,
    /* i32          */ kDeny,
    /* i64          */ kDeny,
    /* u8           */ kDeny,
    /* u16          */ kDeny,
    /* u32          */ kDeny,
    /* u64          */ kDeny,
    /* f32          */ kDeny,
    /* f64          */ kDeny,
    /* str          */ kDeny,
    /* ref + class  */ kBoxingTo,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kKeep,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [val + struct] */ {
    /* void         */ kDeny,
    /* byte         */ kDeny,
    /* word         */ kDeny,
    /* dword        */ kDeny,
    /* qword        */ kDeny,
    /* i8           */ kDeny,
    /* i16          */ kDeny,
    /* i32          */ kDeny,
    /* i64          */ kDeny,
    /* u8           */ kDeny,
    /* u16          */ kDeny,
    /* u32          */ kDeny,
    /* u64          */ kDeny,
    /* f32          */ kDeny,
    /* f64          */ kDeny,
    /* str          */ kDeny,
    /* ref + class  */ kDeny,
    /* ref + struct */ kBoxingTo,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kBitCastTo,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [val + iface] */ {
    /* void         */ kDeny,
    /* byte         */ kDeny,
    /* word         */ kDeny,
    /* dword        */ kDeny,
    /* qword        */ kDeny,
    /* i8           */ kDeny,
    /* i16          */ kDeny,
    /* i32          */ kDeny,
    /* i64          */ kDeny,
    /* u8           */ kDeny,
    /* u16          */ kDeny,
    /* u32          */ kDeny,
    /* u64          */ kDeny,
    /* f32          */ kDeny,
    /* f64          */ kDeny,
    /* str          */ kDeny,
    /* ref + class  */ kIfaceToRef,
    /* ref + struct */ kIfaceToRef,
    /* ref + iface  */ kKeep,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kBitCastTo,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [val + fun] */ {
    /* void         */ kDeny,
    /* byte         */ kDeny,
    /* word         */ kDeny,
    /* dword        */ kDeny,
    /* qword        */ kDeny,
    /* i8           */ kDeny,
    /* i16          */ kDeny,
    /* i32          */ kDeny,
    /* i64          */ kDeny,
    /* u8           */ kDeny,
    /* u16          */ kDeny,
    /* u32          */ kDeny,
    /* u64          */ kDeny,
    /* f32          */ kDeny,
    /* f64          */ kDeny,
    /* str          */ kDeny,
    /* ref + class  */ kDeny,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [val + array] */ {
    /* void         */ kDeny,
    /* byte         */ kDeny,
    /* word         */ kDeny,
    /* dword        */ kDeny,
    /* qword        */ kDeny,
    /* i8           */ kDeny,
    /* i16          */ kDeny,
    /* i32          */ kDeny,
    /* i64          */ kDeny,
    /* u8           */ kDeny,
    /* u16          */ kDeny,
    /* u32          */ kDeny,
    /* u64          */ kDeny,
    /* f32          */ kDeny,
    /* f64          */ kDeny,
    /* str          */ kDeny,
    /* ref + class  */ kDeny,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
    /* from [val + chan] */ {
    /* void         */ kDeny,
    /* byte         */ kDeny,
    /* word         */ kDeny,
    /* dword        */ kDeny,
    /* qword        */ kDeny,
    /* i8           */ kDeny,
    /* i16          */ kDeny,
    /* i32          */ kDeny,
    /* i64          */ kDeny,
    /* u8           */ kDeny,
    /* u16          */ kDeny,
    /* u32          */ kDeny,
    /* u64          */ kDeny,
    /* f32          */ kDeny,
    /* f64          */ kDeny,
    /* str          */ kDeny,
    /* ref + class  */ kDeny,
    /* ref + struct */ kDeny,
    /* ref + iface  */ kDeny,
    /* ref + fun    */ kDeny,
    /* ref + array  */ kDeny,
    /* ref + chan   */ kDeny,
    /* val + class  */ kDeny,
    /* val + struct */ kDeny,
    /* val + iface  */ kDeny,
    /* val + fun    */ kDeny,
    /* val + array  */ kDeny,
    /* val + chan   */ kDeny,
    },
}; // static const ConversionHint kConversionRulers


static inline int ComputeConversionRuleIndex(const Type &type) {
    if (type.kind() == Type::kReference) {
        return kConversionRulersRefOffset + static_cast<int>(type.model()->declaration());
    }
    if (type.kind() == Type::kValue) {
        return kConversionRulersValOffset + static_cast<int>(type.model()->declaration());
    }
    return static_cast<int>(type.kind());
}

ConversionHint GetConversionHint(const Type &dest, const Type &src) {
    auto from_index = ComputeConversionRuleIndex(src);
    auto to_index = ComputeConversionRuleIndex(dest);
    return kConversionRules[from_index][to_index];
}

} // namespace ir

} // namespace yalx
