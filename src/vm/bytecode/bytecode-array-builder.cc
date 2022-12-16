#include "vm/bytecode/bytecode-array-builder.h"

namespace yalx {

namespace vm {

BytecodeArrayBuilder::BytecodeArrayBuilder(base::Arena *arena)
: arena_(DCHECK_NOTNULL(arena))
, bytecodes_(arena) {
}

} // namespace vm

} // namespace yalx
