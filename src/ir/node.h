#pragma once
#ifndef YALX_IR_NODE_H_
#define YALX_IR_NODE_H_

#include "base/checking.h"
#include "base/arena.h"

namespace yalx {
namespace base {
class ArenaString;
} // namespace base
namespace ir {

//    +- Operand
//        +- Argument
//        +- StackAllocate
//        +- HeapAllocate
//        +- StackLoad
//        +- StackStore
//        +- GlobalLoad
//        +- GlobalStore
//        +- CallRuntime
//        +- CallDirect
//        +- CallIndirect
//        +- CallVirtual
//        +- Arithmetic<N>
//            +- I8{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Sar}
//            +- U8{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Shr}
//            +- I16{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Sar}
//            +- U16{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Shr}
//            +- I32{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Sar}
//            +- U32{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Shr}
//            +- I64{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Sar}
//            +- U64{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Shr}
//            +- F32{Add/Sub/Mul/Div}
//            +- F64{Add/Sub/Mul/Div}

#define DECLARE_ALL_NODES(V) \
    DECLARE_IR_NODES(V) \

#define DECLARE_IR_NODES(V) \
    V(Argument)

#define DEFINE_PREDECL_CLASSES(name) class name;
    DECLARE_ALL_NODES(DEFINE_PREDECL_CLASSES)
#undef DEFINE_PREDECL_CLASSES

using String = base::ArenaString;

class SourceInfo final {
public:
    SourceInfo(): SourceInfo(-1) {}
    explicit SourceInfo(int index): index_(index) {}
    
    int value() const { assert(valid()); return index_; }
    bool valid() const { index_ > 0; }
private:
    int index_ = -1;
}; // class SourceIndex

class Node : public base::ArenaObject {
public:
    enum Kind {
#define DEFINE_ENUM(name) k##name,
        DECLARE_ALL_NODES(DEFINE_ENUM)
#undef DEFINE_ENUM
        kMaxKinds,
    }; // enum Kind
    
#define DEFINE_METHODS(name) \
bool Is##name() const { return kind() == k##name; } \
inline name *As##name(); \
inline const name *As##name() const;
    DECLARE_ALL_NODES(DEFINE_METHODS)
#undef DEFINE_METHODS
    
    DEF_VAL_GETTER(Kind, kind);
    DEF_VAL_GETTER(SourceInfo, source_position);
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Node);
protected:
    Node(Kind kind, SourceInfo source_position);
    
private:
    const Kind kind_;
    const SourceInfo source_position_;
}; // class Node

} // namespace ir

} // namespace yalx

#endif // YALX_IR_NODE_H_
