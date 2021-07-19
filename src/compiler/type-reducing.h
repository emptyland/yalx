#pragma once
#ifndef YALX_COMPILER_TYPE_REDUCING_H_
#define YALX_COMPILER_TYPE_REDUCING_H_

#include "compiler/node.h"
#include "base/status.h"
#include "base/arena.h"

namespace yalx {

namespace cpl {

class SyntaxFeedback;

base::Status ReducePackageDependencesType(Package *entry, base::Arena *arena, SyntaxFeedback *error_feedback);

} // namespace cpl

} // namespace yalx

#endif // YALX_COMPILER_TYPE_REDUCING_H_
