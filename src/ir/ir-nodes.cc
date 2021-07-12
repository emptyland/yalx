#include "ir/ir-nodes.h"

namespace yalx {

namespace ir {

Operand::Operand(Kind kind, SourceInfo source_position)
    : Node(kind, source_position) {
}

Argument::Argument(int index, SourceInfo source_position)
    : Operand(Node::kArgument, source_position)
    , index_(index) {
    assert(index >= 0);
}

} // namespace ir

} // namespace yalx
