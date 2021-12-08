#include "compiler/constants.h"

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

} // namespace cpl

} // namespace yalx
