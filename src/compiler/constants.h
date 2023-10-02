#pragma once
#ifndef YALX_COMPILER_CONSTANTS_H_
#define YALX_COMPILER_CONSTANTS_H_

#include "base/base.h"

namespace yalx::cpl {

    enum class CastingRule {
        DENY,  // deny casting
        ALLOW, // allow casting directly
        ALLOW_UNBOX, // allow casting unbox first
        ALLOW_TYPING, // allow casting check type first
        ALLOW_TYPING_CONCEPT, // allow casting check type first and koncept interface
        ALLOW_OPTION_TEST, // allow casting and option testing
        PROTOTYPE, // allow casting check fun prototype first
        ELEMENT, // check element type first
        ELEMENT_IN_OUT, // check element type and in/out
        I8_U8_CHAR_ARRAY_ONLY, // only to i8[], u8[], char[]
        OPTION_VALUE,
        SELF_ONLY, // only to itself
        CONCEPT, // check koncept interface
        CHILD_CLASS_ONLY, // child class only
        CLASS_BASE_OF,
    }; // class enum CastingRule

    class Constants final {
    public:

        static int ReduceNumberType(int lhs, int rhs);

        static CastingRule HowToCasting(int dest, int src);

        static const char *kPrimitiveTypeClassNames[];
        DISALLOW_ALL_CONSTRUCTORS(Constants);
    }; // class Constants

    constexpr static char kYalxName[] = "yalx";
    constexpr static char kAnnoCompilerName[] = "Compiler";
    constexpr static char kAnnoNativeHandleProperty[] = "nativeHandle";
    constexpr static char kAnnoNativeStubNameProperty[] = "nativeStubName";

    constexpr static char kLangPackageFullName[] = "yalx/lang:lang";
    constexpr static char kAnyClassName[] = "Any";
    constexpr static char kAnyClassFullName[] = "yalx/lang:lang.Any";
    constexpr static char kAnyArrayFullName[] = "yalx/lang:lang.Any[]";
    constexpr static char kThrowableClassFullName[] = "yalx/lang:lang.Throwable";

    constexpr static char kOptionalClassName[] = "Optional";
    constexpr static char kOptionalClassFullName[] = "yalx/lang:lang.Optional";

    constexpr static char kI32ClassFullName[] = "yalx/lang:lang.I32";
    constexpr static char kU32ClassFullName[] = "yalx/lang:lang.U32";

    constexpr static char kStringClassName[] = "String";
    constexpr static char kStringClassFullName[] = "yalx/lang:lang.String";
    constexpr static char kThrowableClassName[] = "Throwable";

    constexpr static char kModuleInitFunName[] = "$init";
    constexpr static char kModuleInitFunProtoName[] = "fun ()->void";

    constexpr static char kObjectShadowClassPostfix[] = "$ShadowClass";
    constexpr static char kPrimaryConstructorPostfix[] = "$constructor";

    constexpr static char kCompareToFunName[] = "compareTo";

    constexpr static char kArgsName[] = "args";
    constexpr static char kThisName[] = "this";
    constexpr static char kCalleeName[] = "callee";

    constexpr static int kMaxArrayInitializerDims = 99;
} // namespace yalx

#endif // YALX_COMPILER_CONSTANTS_H_
