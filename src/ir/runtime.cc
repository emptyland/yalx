#include "ir/runtime.h"

namespace yalx {

namespace ir {

static char const *const kRuntimeNames[] = {
#define DEFINE_NAME(name) #name,
    DECL_RUNTIME_IDS(DEFINE_NAME)
#undef  DEFINE_NAME
};

const char *RuntimeId::ToString() const { return kRuntimeNames[value]; }

} // namespace ir

} // namespace yalx
