#pragma once
#ifndef YALX_IR_IR_NODES_H_
#define YALX_IR_IR_NODES_H_

#include "ir/node.h"

namespace yalx {

namespace ir {

class Operand : public Node {
protected:
    Operand(Kind kind, SourceInfo source_position);
}; // class Operand

class Argument : public Operand {
public:
    Argument(int index, SourceInfo source_position);
    
    DEF_VAL_GETTER(int, index);
private:
    int index_;
}; // class Argument

class Type {
    
    uint16_t integral_: 1;
    uint16_t floating_: 1;
    uint16_t signed__: 1;
    uint16_t reference_: 1;
    uint16_t bytes_;
};

#define DEFINE_METHODS(name) \
inline name *Node::As##name() { return static_cast<name *>(this); } \
inline const name *Node::As##name() const { return static_cast<const name *>(this); }
    DECLARE_IR_NODES(DEFINE_METHODS)
#undef DEFINE_METHODS

} // namespace ir

} // namespace yalx

#endif // YALX_IR_IR_NODES_H_
