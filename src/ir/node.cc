#include "ir/node.h"

namespace yalx {

namespace ir {

Node::Node(Kind kind, SourceInfo source_position)
    : kind_(kind)
    , source_position_(source_position) {
}

} // namespace ir

} // namespace yalx
