#include "backend/frame.h"


namespace yalx {

namespace backend {

Frame::Frame(base::Arena *arena, ir::Function *fun)
: virtual_registers_(arena)
, fun_(fun) {
    
}


} // namespace backend

} // namespace yalx
