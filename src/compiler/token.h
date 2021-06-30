#pragma once
#ifndef YALX_COMPILER_TOKEN_H_
#define YALX_COMPILER_TOKEN_H_

#include "compiler/source-position.h"
#include "base/status.h"
#include "base/checking.h"
#include "base/base.h"

namespace yalx {
namespace base {
class ArenaString;
} // namespace base
namespace cpl {

using String = base::ArenaString;

#define DECLARE_ALL_TOKEN(V) \
    V(EOF, nullptr, none) \
    V(Error, nullptr, none) \
    V(Identifier, nullptr, text) \
    V(StringLine, nullptr, text) \
    V(StringBlock, nullptr, text) \
    V(StringTempletePrefix, nullptr, text) \
    V(StringTempletePart, nullptr, text) \
    V(StringTempleteSuffix, nullptr, text) \
    V(StringTempleteExpressBegin, "{", text) \
    V(StringTempleteExpressEnd, "}", text) \
    V(IntVal, nullptr, i64) \
    V(UIntVal, nullptr, u64) \
    V(F32Val, nullptr, f32) \
    V(F64Val, nullptr, f64) \
    V(Ref, nullptr, none) \
    V(Pair, nullptr, none) \
    V(Comma, ",", none) \
    V(Plus, "+", none) \
    V(2Plus, "++", none) \
    V(PlusEqual, "+=", none) \
    V(Minus, "-", none) \
    V(2Minus, "--", none) \
    V(MinusEqual, "-=", none) \
    V(Star, "*", none) \
    V(Div, "/", none) \
    V(Percent, "%", none) \
    V(LBrace, "{", none) \
    V(RBrace, "}", none) \
    V(LParen, "(", none) \
    V(RParen, ")", none) \
    V(LBrack, "[", none) \
    V(RBrack, "]", none) \
    V(Wave, "~", none) \
    V(Dot, ".", none) \
    V(BitwiseAnd, "&", none) \
    V(BitwiseOr, "|", none) \
    V(BitwiseXor, "^", none) \
    V(And, "&&", none) \
    V(Or, "||", none) \
    V(Not, "!", none) \
    V(More, "..", none) \
    V(Vargs, "...", none) \
    V(Colon, ":", none) \
    V(Semi, ";", none) \
    V(2Colon, "::", none) \
    V(Less, "<", none) \
    V(LessEqual, "<=", none) \
    V(LShift, "<<", none) \
    V(LArrow, "<-", none) \
    V(Greater, ">", none) \
    V(GreaterEqual, ">=", none) \
    V(RShift, ">>", none) \
    V(RArrow, "->", none) \
    V(Equal, "==", none) \
    V(NotEqual, "!=", none) \
    V(Assign, "=", none) \
    V(Dollar, "$", none) \
    V(AtOutlined, "@", none) \
    DECLARE_KEYWORDS_TOKEN(V)

#define DECLARE_KEYWORDS_TOKEN(V) \
    V(True, "true", none) \
    V(False, "false", none) \
    V(EmptyVal, "Empty", none) \
    V(UnitVal, "Unit", none) \
    V(Val, "val", none) \
    V(Var, "var", none) \
    V(Volatile, "volatile", none) \
    V(Native, "native", none) \
    V(Struct, "struct", none) \
    V(Class, "class", none) \
    V(Object, "object", none) \
    V(Interface, "interface", none) \
    V(Fun, "fun", none) \
    V(Chan, "chan", none) \
    V(In, "in", none) \
    V(Out, "out", none) \
    V(Run, "run", none) \
    V(Spwan, "spwan", none) \
    V(Import, "import", none) \
    V(Package, "package", none) \
    V(Public, "public", none) \
    V(Protected, "protected", none) \
    V(Private, "private", none) \
    V(Is, "is", none) \
    V(As, "as", none) \
    V(If, "if", none) \
    V(Else, "else", none) \
    V(When, "when", none) \
    V(While, "while", none) \
    V(Unless, "unless", none) \
    V(Until, "until", none) \
    V(For, "for", none) \
    V(Return, "return", none) \
    V(Break, "break", none) \
    V(Continue, "continue", none) \
    V(Throw, "throw", none) \
    V(Try, "try", none) \
    V(Catch, "catch", none) \
    V(Finally, "finally", none) \
    DECLARE_BASE_TYPE_TOKEN(V)

#define DECLARE_BASE_TYPE_TOKEN(V) \
    V(Unit, "unit", none) \
    V(Bool, "bool", none) \
    V(I8, "i8", none) \
    V(U8, "u8", none) \
    V(I16, "i16", none) \
    V(U16, "u16", none) \
    V(I32, "i32", none) \
    V(U32, "u32", none) \
    V(I64, "i64", none) \
    V(U64, "u64", none) \
    V(Int, "int", none) \
    V(UInt, "uint", none) \
    V(F32, "f32", none) \
    V(F64, "f64", none) \
    V(Any, "any", none) \
    V(String, "string", none)

class Token final {
public:
    enum Kind {
    #define DEFINE_ENUM(name, ...) k##name,
        DECLARE_ALL_TOKEN(DEFINE_ENUM)
    #undef  DEFINE_ENUM
        kMax,
    }; // enum Kind
    
    Token(Kind kind, const SourcePosition &position)
        : kind_(kind)
        , source_position_(position) {
        none_val_ = nullptr;
    }
    
    template<class T>
    inline const Token &With(T val) {
        Setter<T>{}.Set(this, val);
        return *this;
    }

    DEF_VAL_GETTER(Kind, kind);
    DEF_PTR_PROP_RW(const String, text_val);
    DEF_VAL_GETTER(SourcePosition, source_position);
    DEF_VAL_PROP_RW(float, f32_val);
    DEF_VAL_PROP_RW(double, f64_val);
    DEF_VAL_PROP_RW(int64_t, i64_val);
    DEF_VAL_PROP_RW(uint64_t, u64_val);
    
    bool Is(Kind expect) const { return kind() == expect; }
    bool IsNot(Kind expect) const { return !Is(expect); }
    
    struct NamePair {
        const char *const name;
        const char *const literal;
    }; // struct NamePair

    const NamePair &name_pair() const { return GetNamePair(kind_); }
    
    std::string ToString() const { return ToString(kind_); }
    
    // Test: Is a keyword?
    static Kind IsKeyword(std::string_view text);
    
    static const NamePair &GetNamePair(Kind kind) {
        assert(static_cast<int>(kind) >= 0 && static_cast<int>(kind) < kMax);
        return kNameTable[kind];
    }
    
    static std::string ToString(Kind kind) {
        const NamePair &pair = GetNamePair(kind);
        std::string buf(pair.name);
        if (pair.literal) {
            buf.append(" `").append(pair.literal).append("'");
        }
        return buf;
    }
private:
    template<class T>
    struct Setter {
        void Set(Token *token, T value) { token->none_val_ = nullptr; }
    };
    
    template<>
    struct Setter<const String *> {
        void Set(Token *token, const String *value) { token->text_val_ = value; }
    };
    
    template<>
    struct Setter<int64_t> {
        void Set(Token *token, int64_t value) { token->i64_val_ = value; }
    };
    
    template<>
    struct Setter<uint64_t> {
        void Set(Token *token, uint64_t value) { token->u64_val_ = value; }
    };
    
    template<>
    struct Setter<float> {
        void Set(Token *token, float value) { token->f32_val_ = value; }
    };
    
    template<>
    struct Setter<double> {
        void Set(Token *token, double value) { token->f64_val_ = value; }
    };
    
    static NamePair kNameTable[kMax];
    
    Kind kind_;
    SourcePosition source_position_;
    union {
        const String *text_val_;
        float    f32_val_;
        double   f64_val_;
        uint64_t u64_val_;
        int64_t  i64_val_;
        void    *none_val_;
    };
}; // class Token

} // namespace cpl

} // namespace yalx

#endif // YALX_COMPILER_TOKEN_H_
