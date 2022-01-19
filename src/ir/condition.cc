#include "ir/condition.h"

namespace yalx {

namespace ir {

static char const *const kIConditionNames[] = {
#define DEFINE_NAME(name) #name,
    DECL_INTEGRAL_CONDITIONS(DEFINE_NAME)
#undef  DEFINE_NAME
};

static char const *const kFConditionNames[] = {
#define DEFINE_NAME(name) #name,
    DECL_FLOATING_CONDITIONS(DEFINE_NAME)
#undef  DEFINE_NAME
};

const char *IConditionId::ToString() const { return kIConditionNames[value]; }
const char *FConditionId::ToString() const { return kFConditionNames[value]; }

} // namespace ir

} // namespace yalx
