#include "compiler/constants.h"
#include "compiler/ast.h"

namespace yalx {

namespace cpl {

const char *Constants::kPrimitiveTypeClassNames[] = {
    nullptr,        //    V(unit)
    kAnyClassName,  //    V(any)
    "Bool",         //    V(bool)
    "Char",         //    V(char)
    "I8",           //    V(i8)
    "U8", //    V(u8)
    "I16", //    V(i16)
    "U16", //    V(u16)
    "I32", //    V(i32)
    "U32", //    V(u32)
    "I64", //    V(i64)
    "U64", //    V(u64)
    "F32", //    V(f32)
    "F64", //    V(f64)
    "String", //    V(string)
    nullptr, //    V(channel)
    nullptr, //    V(array)
    nullptr, //    V(class)
    nullptr, //    V(struct)
    nullptr, //    V(interface)
    nullptr, //    V(function)
    nullptr, //    V(symbol)
};

constexpr static Type::Primary kErrTag = Type::kMaxTypes;

#define PREIFX_PADDING  kErrTag, kErrTag, kErrTag, kErrTag
#define POSTFIX_PADDING kErrTag, kErrTag, kErrTag, kErrTag, kErrTag, kErrTag, kErrTag, kErrTag
#define ALL_PADDING     PREIFX_PADDING, \
                        kErrTag, kErrTag, kErrTag, kErrTag, kErrTag, kErrTag, kErrTag, kErrTag, kErrTag, kErrTag, \
                        POSTFIX_PADDING
#define D(name)         Type::kType_##name


static const Type::Primary kTypeReducingRules[Type::kMaxTypes][Type::kMaxTypes] = {
    /* x */    {ALL_PADDING},
    /* x */    {ALL_PADDING},
    /* x */    {ALL_PADDING},
    /* x */    {ALL_PADDING},
    /* i8 */   {PREIFX_PADDING, D(i8),  D(u8),  D(i16), D(u16), D(i32), D(u32), D(i64), D(u64), D(f32), D(f64), POSTFIX_PADDING},
    /* u8 */   {PREIFX_PADDING, D(u8),  D(u8),  D(i16), D(u16), D(i32), D(u32), D(i64), D(u64), D(f32), D(f64), POSTFIX_PADDING},
    /* i16 */  {PREIFX_PADDING, D(i16), D(i16), D(i16), D(u16), D(i32), D(u32), D(i64), D(u64), D(f32), D(f64), POSTFIX_PADDING},
    /* u16 */  {PREIFX_PADDING, D(u16), D(u16), D(u16), D(u16), D(i32), D(u32), D(i64), D(u64), D(f32), D(f64), POSTFIX_PADDING},
    /* i32 */  {PREIFX_PADDING, D(i32), D(i32), D(i32), D(i32), D(i32), D(u32), D(i64), D(u64), D(f32), D(f64), POSTFIX_PADDING},
    /* u32 */  {PREIFX_PADDING, D(u32), D(u32), D(u32), D(u32), D(u32), D(u32), D(i64), D(u64), D(f32), D(f64), POSTFIX_PADDING},
    /* i64 */  {PREIFX_PADDING, D(i64), D(i64), D(i64), D(i64), D(i64), D(i64), D(i64), D(u64), D(f32), D(f64), POSTFIX_PADDING},
    /* u64 */  {PREIFX_PADDING, D(u64), D(u64), D(u64), D(u64), D(u64), D(u64), D(u64), D(u64), D(f32), D(f64), POSTFIX_PADDING},
    /* f32 */  {PREIFX_PADDING, D(f32), D(f32), D(f32), D(f32), D(f32), D(f32), D(f32), D(f32), D(f32), D(f64), POSTFIX_PADDING},
    /* f64 */  {PREIFX_PADDING, D(f64), D(f64), D(f64), D(f64), D(f64), D(f64), D(f64), D(f64), D(f64), D(f64), POSTFIX_PADDING},
    /* x */    {ALL_PADDING},
    /* x */    {ALL_PADDING},
    /* x */    {ALL_PADDING},
    /* x */    {ALL_PADDING},
    /* x */    {ALL_PADDING},
    /* x */    {ALL_PADDING},
    /* x */    {ALL_PADDING},
    /* x */    {ALL_PADDING},
};


int Constants::ReduceNumberType(int lhs, int rhs) {
    assert(lhs >= 0 && lhs < Type::kMaxTypes);
    assert(rhs >= 0 && rhs < Type::kMaxTypes);
    Type::Primary rv = static_cast<Type::Primary>(kTypeReducingRules[lhs][rhs]);
    assert(rv != Type::kMaxTypes);
    return rv;
}

} // namespace cpl

} // namespace yalx
