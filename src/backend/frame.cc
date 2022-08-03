#include "backend/frame.h"


namespace yalx {

namespace backend {

Frame::Frame(base::Arena *arena)
: virtual_registers_(arena) {
    
}


} // namespace backend

} // namespace yalx
