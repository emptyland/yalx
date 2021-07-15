#include "ir/type.h"
#include "ir/metadata.h"

namespace yalx {

namespace ir {

Type Type::Ref(Model *model) {
    return Type(kModel, (DCHECK_NOTNULL(model)->ReferenceSizeInBytes() << 3), 0, DCHECK_NOTNULL(model));
}

} // namespace ir

} // namespace yalx
