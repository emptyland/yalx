#pragma once
#ifndef YALX_RUNTIME_CHECKING_H_
#define YALX_RUNTIME_CHECKING_H_

#include <assert.h>

#if defined(NDEBUG)

#define DCHECK(e)

#else // defined(NDEBUG)

#define DCHECK(e) assert(e)

#endif // defined(NDEBUG)

#endif // YALX_RUNTIME_CHECKING_H_


