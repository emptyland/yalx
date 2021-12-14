#include "compiler/lexer.h"
#include "compiler/syntax-feedback.h"
#include "base/arena-utils.h"
#include "base/io.h"


namespace yalx {
    
namespace cpl {

base::Status Lexer::SwitchInputFile(const std::string &name, base::SequentialFile *file) {
    input_file_ = DCHECK_NOTNULL(file);
    DCHECK_NOTNULL(error_feedback_)->set_file_name(name);
    scratch_.clear();
    buffered_ = "";
    line_ = 1;
    column_ = 1;
    if (auto rs = input_file_->Available(&available_); rs.fail()) {
        return rs;
    }
    if (auto rs = input_file_->Read(std::min(static_cast<size_t>(available_), kBufferSize), &buffered_, &scratch_);
        rs.fail()) {
        return rs;
    }
    
    buffer_position_ = 0;
    while (!in_string_template_.empty()) {
        in_string_template_.pop();
    }
    in_string_template_.push(kNotTemplate);
    return base::Status::OK();
}

Token Lexer::Next() {
    
    for (;;) {
        int ch = Peek();
        if (in_string_template_.top() == kSimpleTemplate) {
            return MatchSimpleTemplateString();
        } else if (in_string_template_.top() == kExpressionTemplate) {
            if (ch == '}') {
                SourcePosition loc{line_, column_};
                MoveNext();
                in_string_template_.pop();
                return Token(Token::kStringTempleteExpressEnd, loc);
            }
        }

        switch (ch) {
            case 0:
                return Token(Token::kEOF, SourcePosition{line_, column_});
                
            case ' ':
            case '\t':
                MoveNext();
                break;

            case '\r':
                MoveNext();
                if (ch = Peek(); ch == '\n') {
                    MoveNext();
                    line_++;
                    column_ = 1;
                }
                break;

            case '\n':
                MoveNext();
                line_++;
                column_ = 1;
                break;

            case '\"':
                return MatchString(ch, true/*escape*/, false/*block*/);
                
            case '\'':
                return MatchChar();
                
            case '`':
                return MatchString(ch, false/*escape*/, true/*block*/);

            case '(':
                return MatchOne(Token::kLParen);
                
            case ')':
                return MatchOne(Token::kRParen);

            case '[':
                return MatchOne(Token::kLBrack);
                
            case ']':
                return MatchOne(Token::kRBrack);
                
            case '{':
                return MatchOne(Token::kLBrace);
                
            case '}':
                return MatchOne(Token::kRBrace);
                
            case '*':
                return MatchOne(Token::kStar);
                
            case '%':
                return MatchOne(Token::kPercent);
                
            case '~':
                return MatchOne(Token::kWave);
                
            case '^':
                return MatchOne(Token::kBitwiseXor);

            case '_':
                return MatchIdentifier();

            case ',':
                return MatchOne(Token::kComma);
                
            case ';':
                return MatchOne(Token::kSemi);
                
            case '$':
                return MatchOne(Token::kDollar);
                
            case '@':
                return MatchOne(Token::kAtOutlined);
                
            case '!': {
                SourcePosition loc{line_, column_};
                ch = MoveNext();
                if (ch == '=') {
                    loc.SetEnd(line_, column_);
                    MoveNext();
                    return Token(Token::kNotEqual, loc);
                } else if (ch == '!') {
                    loc.SetEnd(line_, column_);
                    MoveNext();
                    return Token(Token::k2Exclamation, loc);
                } else if (ch == '\"') {
                    return MatchString(ch, false/*escape*/, false/*block*/);
                }
                return Token(Token::kNot, loc);
            }
                
            case '?':
                return MatchOne(Token::kQuestion);
                
            case '/': {
                SourcePosition loc{line_, column_};
                ch = MoveNext();
                if (ch == '/') { // `//'
                    while (ch != '\n' && ch != '\r' && ch != 0) {
                        ch = MoveNext();
                    }
                } else {
                    return Token(Token::kDiv, loc);
                }
            } break;
                
            case '|': {
                SourcePosition loc{line_, column_};
                ch = MoveNext();
                if (ch == '|') {
                    loc.SetEnd(line_, column_);
                    MoveNext();
                    return Token(Token::kOr, loc);
                } else {
                    loc.SetEnd(line_, column_);
                    return Token(Token::kBitwiseOr, loc);
                }
            } break;
                
            case '&': {
                SourcePosition loc{line_, column_};
                ch = MoveNext();
                if (ch == '&') {
                    loc.SetEnd(line_, column_);
                    MoveNext();
                    return Token(Token::kAnd, loc);
                } else {
                    loc.SetEnd(line_, column_);
                    return Token(Token::kBitwiseAnd, loc);
                }
            } break;
                
            case '.': {
                SourcePosition loc{line_, column_};
                ch = MoveNext();
                if (ch == '.') {
                    int i;
                    for (i = 0; i < 2; i++) {
                        loc.SetEnd(line_, column_);
                        ch = MoveNext();
                        if (ch != '.') {
                            break;
                        }
                    }
                    return i == 1 ? Token(Token::kVargs, loc) : Token(Token::kMore, loc);
                } else {
                    return Token(Token::kDot, SourcePosition{loc.begin_line(), loc.begin_column()});
                }
            } break;
                
            case ':': {
                SourcePosition loc{line_, column_};
                ch = MoveNext();
                if (ch == ':') {
                    loc.SetEnd(line_, column_);
                    return Token(Token::k2Colon, loc);
                } else {
                    return Token(Token::kColon, SourcePosition{loc.begin_line(), loc.begin_column()});
                }
            } break;

            case '<': {
                int begin_line = line_, begin_row = column_;
                ch = MoveNext();
                if (ch == '=') { // <=
                    MoveNext();
                    return Token(Token::kLessEqual, {begin_line, begin_row, line_, column_});
                } else if (ch == '<') { // <<
                    MoveNext();
                    return Token(Token::kLShift, {begin_line, begin_row, line_, column_});
                } else if (ch == '>') { // <>
                    MoveNext();
                    return Token(Token::kNotEqual, {begin_line, begin_row, line_, column_});
                } else if (ch == '-') { // <-
                    MoveNext();
                    return Token(Token::kLArrow, {begin_line, begin_row, line_, column_});
                } else {
                    return Token(Token::kLess, {begin_line, begin_row});
                }
            } break;
                
            case '>': {
                int begin_line = line_, begin_row = column_;
                ch = MoveNext();
                if (ch == '=') { // >=
                    MoveNext();
                    return Token(Token::kGreaterEqual, {begin_line, begin_row, line_, column_});
                } else if (ch == '>') { // >>
                    MoveNext();
                    return Token(Token::kRShift, {begin_line, begin_row, line_, column_});
                } else {
                    return Token(Token::kGreater, {begin_line, begin_row});
                }
            } break;
                
            case '=': {
                int begin_line = line_, begin_row = column_;
                ch = MoveNext();
                if (ch == '=') {
                    MoveNext();
                    return Token(Token::kEqual, {begin_line, begin_row, line_, column_});
                } else {
                    return Token(Token::kAssign, {begin_line, begin_row});
                }
            } break;

            case '+': {
                int begin_line = line_, begin_row = column_;
                ch = MoveNext();
                if (ch == '=') {
                    MoveNext();
                    return Token(Token::kPlusEqual, {begin_line, begin_row, line_, column_});
                } else if (ch == '+') {
                    MoveNext();
                    return Token(Token::k2Plus, {begin_line, begin_row, line_, column_});
                } else if (::isdigit(ch)) {
                    return MatchNumber(1/*sign*/, line_, column_);
                } else {
                    return Token(Token::kPlus, {begin_line, begin_row});
                }
            } break;
                
            case '-': {
                int begin_line = line_, begin_row = column_;
                ch = MoveNext();
                if (ch == '=') { // -=
                    MoveNext();
                    return Token(Token::kMinusEqual, {begin_line, begin_row, line_, column_});
                } else if (ch == '-') { // --
                    MoveNext();
                    return Token(Token::k2Minus, {begin_line, begin_row, line_, column_});
                } else if (ch == '>') { // ->
                    MoveNext();
                    return Token(Token::kRArrow, {begin_line, begin_row, line_, column_});
                } else if (::isdigit(ch)) {
                    return MatchNumber(-1/*sign*/, line_, column_);
                } else {
                    return Token(Token::kMinus, {begin_line, begin_row});
                }
            } break;
                
            default:
                if (ch == -1) {
                    return Token(Token::kError, {line_, column_});
                }
                if (::isdigit(ch)) {
                    return MatchNumber(0/*sign*/, line_, column_);
                }
                if (IsUtf8Prefix(ch) && !IsTermChar(ch)) {
                    return MatchIdentifier();
                }
                error_feedback_->Printf({line_, column_}, "Unexpected: %c(%x)", ch, ch);
                return Token(Token::kError, {line_, column_});
        }
    }
}

Token Lexer::MatchIdentifier() {
    SourcePosition loc{line_, column_};

    std::string buf;
    for (;;) {
        int ch = Peek();
        if (ch == -1) {
            error_feedback_->Printf(loc, "Unexpected identifier character, expected %x", ch);
            return Token(Token::kError, loc);
        } else if (IsTermChar(ch)) {
            Token::Kind keyword = Token::IsKeyword(buf);
            if (keyword != Token::kError) {
                return Token(keyword, loc);
            }
            const String *asts = String::New(arena_, buf.data(), buf.size());
            return Token(Token::kIdentifier, loc).With(asts);
        } else {
            loc.SetEnd(line_, column_);
            if (!MatchUtf8Character(&buf)) {
                return Token(Token::kError, {line_, column_});
            }
        }
    }
}

Token Lexer::MatchNumber(int sign, int line, int row) {
    SourcePosition loc{line, row};
    
    int ch = Peek();
    if (sign == 0) {
        if (ch == '-') {
            MoveNext();
            sign = -1;
        } else if (ch == '+') {
            MoveNext();
            sign = 1;
        } else {
            sign = 1;
        }
    }

    std::string buf;
    int base = 10;
    if (Peek() == '0') {
        ch = MoveNext();
        if (ch == 'x' || ch == 'X') {
            MoveNext();
            base = 16;
            buf.append("0x");
        } else {
            if (::isdigit(Peek())) {
                base = 8;
            }
            buf.append("0");
        }
    }

    int has_e = 0;
    Token::Kind type = Token::kIntVal;
    for (;;) {
        ch = Peek();
        if (ch == -1) {
            error_feedback_->Printf(loc, "I/O error");
            return Token(Token::kError, loc);
        } else if (::isdigit(ch)) {
            if (base == 8 && ch >= '8' && ch <= '9') {
                return OneCharacterError(line_, column_, "Unexpected number character: %c", ch);
            }
            MoveNext();
            buf.append(1, ch);
        } else if ((type != Token::kF32Val && type != Token::kF64Val) &&
                   ((ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))) {
            if (base != 16/* || type == Token::kF32Val || type == Token::kF64Val*/) {
                return OneCharacterError(line_, column_, "Unexpected number character: %c", ch);
            }
            MoveNext();
            buf.append(1, ch);
        } else if (ch == 'e') {
            if (has_e++ > 0) {
                return OneCharacterError(line_, column_, "Duplicated exp `e' in floating literal");
            }
            if (base != 10) {
                return OneCharacterError(line_, column_, "Unexpected integral character: %c", ch);
            }
            MoveNext();
            buf.append(1, ch);
        } else if (ch == '.') {
            if (type == Token::kF32Val || type == Token::kF64Val) {
                return OneCharacterError(line_, column_, "Duplicated dot `.' in floating literal");
            }
            if (base != 10) {
                return OneCharacterError(line_, column_, "Unexpected integral character: %c", ch);
            }
            type = Token::kF32Val;
            buf.append(1, ch);
            MoveNext();
        } else if (ch == 'f') {
            if (base != 10) {
                return OneCharacterError(line_, column_, "Unexpected integral character: %c", ch);
            }
            ch = MoveNext();
            if (!IsTermChar(ch)) {
                return OneCharacterError(line_, column_, "Incorrect number literal suffix: f%x", ch);
            }
            type = Token::kF32Val;
        } else if (ch == 'd') {
            ch = MoveNext();
            if (!IsTermChar(ch)) {
                return OneCharacterError(line_, column_, "Incorrect number literal suffix: d%x", ch);
            }
            type = Token::kF64Val;
        } else if (ch == 'i' || ch == 'u') {
            int sign_ch = ch;
            if (type == Token::kF32Val || type == Token::kF64Val) {
                return OneCharacterError(line_, column_, "Incorrect integral suffix in floating number");
            }

            ch = MoveNext();
            type = sign_ch == 'i' ? Token::kIntVal : Token::kUIntVal;
        } else if (IsTermChar(ch)) {
            loc.SetEnd(line_, column_);
            break;
        } else {
            return OneCharacterError(line_, column_, "Unexpected number character: %c", ch);
        }
    }
    
    uint64_t val = 0;
    switch (type) {
        case Token::kIntVal:
        case Token::kUIntVal: {
            int rv = 0;
            if (base == 8) {
                rv = base::ParseO64(buf.c_str() + 1, buf.length() - 1, &val);
            } else if (base == 10) {
                int64_t tmp;
                if (type == Token::kIntVal) {
                    rv = base::ParseI64(buf.c_str(), &tmp);
                    val = tmp;
                } else {
                    rv = base::ParseU64(buf.c_str(), &val);
                }
                
            } else if (base == 16) {
                rv = base::ParseH64(buf.c_str() + 2, buf.length() - 2, &val);
            }
            assert(rv >= 0);
            if (rv > 0) {
                error_feedback_->Printf(loc, "Integral number: %s overflow", buf.data());
                return Token(Token::kError, loc);
            }
        } break;

        case Token::kF32Val: {
            float f32 = ::atof(buf.data());
            return Token(Token::kF32Val, loc).With(sign < 0 ? -f32 : f32);
        } break;

        case Token::kF64Val: {
            double f64 = ::atof(buf.data());
            return Token(Token::kF64Val, loc).With(sign < 0 ? -f64 : f64);
        } break;
            
        default:
            UNREACHABLE();
            break;
    }

    switch (type) {
        case Token::kIntVal: {
            auto test = sign < 0 ? -static_cast<int64_t>(val) : static_cast<int64_t>(val);
            return Token(type, loc).With(test);
        } break;
            
        case Token::kUIntVal: {
            auto test = sign < 0 ? -static_cast<uint64_t>(val) : static_cast<uint64_t>(val);
            return Token(type, loc).With(test);
        } break;

        default:
            UNREACHABLE();
            break;
    }
    return Token(Token::kError, loc);
}

Token Lexer::MatchSimpleTemplateString() {
    SourcePosition loc{line_, column_};
    std::string buf;
    for (;;) {
        int ch = Peek();
        if (ch == '\"') {
            loc.SetEnd(line_, column_);
            MoveNext();
            
            in_string_template_.pop();
            const String *asts = String::New(arena_, buf.data(), buf.size());
            return Token(Token::kStringTempleteSuffix, loc).With(asts);
        } else if (ch == '$') {
            loc.SetEnd(line_, column_);
            if (!buf.empty()) {
                const String *asts = String::New(arena_, buf.data(), buf.size());
                return Token(Token::kStringTempletePart, loc).With(asts);
            }
            ch = MoveNext();
            if (ch == '{') {
                loc.SetEnd(line_, column_);
                MoveNext();
                in_string_template_.push(kExpressionTemplate);
                assert(buf.empty());
                return Token(Token::kStringTempleteExpressBegin, loc);
            } else {
                return MatchIdentifier();
            }
        } else if (ch == '\\') {
            if (!MatchEscapeCharacter(&buf)) {
                return Token(Token::kError, {line_, column_});
            }
        } else if (ch == '\r' || ch == '\n') {
            loc.SetEnd(line_, column_);
            error_feedback_->Printf(loc, "Unexpected string term, expected '\\r' '\\n'");
            return Token(Token::kError, loc);
        } else {
            if (!MatchUtf8Character(&buf)) {
                return Token(Token::kError, {line_, column_});
            }
        }
    }
    return Token(Token::kError, {0, 0});
}

Token Lexer::MatchChar() {
    SourcePosition loc{line_, column_};
    MoveNext(); // skip `''
    std::string buf;
    if (!MatchUtf8Character(&buf)) {
        return Token(Token::kError, {0, 0});
    }
    
    // Use UTF-32 BE
    char32_t val = 0;
    switch (buf.size()) {
        case 1:
            val = buf[0];
            break;
        case 2:
//            val = (static_cast<char32_t>(buf[0]) & 0b11111)
//            | ((static_cast<char32_t>(buf[1]) & 0b111111) << 5);
            val = ((static_cast<char32_t>(buf[0]) & 0b11111) << 6)
            | (static_cast<char32_t>(buf[1]) & 0b111111);
            break;
        case 3:
//            val = (static_cast<char32_t>(buf[0]) & 0b1111)
//            | ((static_cast<char32_t>(buf[1]) & 0b111111) << 4)
//            | ((static_cast<char32_t>(buf[2]) & 0b111111) << 10);
            val = ((static_cast<char32_t>(buf[0]) & 0b1111) << 12)
            | ((static_cast<char32_t>(buf[1]) & 0b111111) << 6)
            | (static_cast<char32_t>(buf[2]) & 0b111111);
            break;
        case 4:
//            val = (static_cast<char32_t>(buf[0]) & 0b111)
//            | ((static_cast<char32_t>(buf[1]) & 0b111111) << 3)
//            | ((static_cast<char32_t>(buf[2]) & 0b111111) << 9)
//            | ((static_cast<char32_t>(buf[3]) & 0b111111) << 15);
            val = ((static_cast<char32_t>(buf[0]) & 0b111) << 18)
            | ((static_cast<char32_t>(buf[1]) & 0b111111) << 12)
            | ((static_cast<char32_t>(buf[2]) & 0b111111) << 6)
            | ((static_cast<char32_t>(buf[3]) & 0b111111));
            break;
        case 5:
//            val = (static_cast<char32_t>(buf[0]) & 0b11)
//            | ((static_cast<char32_t>(buf[1]) & 0b111111) << 2)
//            | ((static_cast<char32_t>(buf[2]) & 0b111111) << 8)
//            | ((static_cast<char32_t>(buf[3]) & 0b111111) << 14)
//            | ((static_cast<char32_t>(buf[4]) & 0b111111) << 20);
            val = ((static_cast<char32_t>(buf[0]) & 0b11) << 24)
            | ((static_cast<char32_t>(buf[1]) & 0b111111) << 18)
            | ((static_cast<char32_t>(buf[2]) & 0b111111) << 12)
            | ((static_cast<char32_t>(buf[3]) & 0b111111) << 6)
            | (static_cast<char32_t>(buf[4]) & 0b111111);
            break;
        case 6:
//            val = (static_cast<char32_t>(buf[0]) & 0b1)
//            | ((static_cast<char32_t>(buf[1]) & 0b111111) << 1)
//            | ((static_cast<char32_t>(buf[2]) & 0b111111) << 7)
//            | ((static_cast<char32_t>(buf[3]) & 0b111111) << 13)
//            | ((static_cast<char32_t>(buf[4]) & 0b111111) << 19)
//            | ((static_cast<char32_t>(buf[5]) & 0b111111) << 25);
            val = ((static_cast<char32_t>(buf[0]) & 0b1) << 30)
            | ((static_cast<char32_t>(buf[1]) & 0b111111) << 24)
            | ((static_cast<char32_t>(buf[2]) & 0b111111) << 18)
            | ((static_cast<char32_t>(buf[3]) & 0b111111) << 12)
            | ((static_cast<char32_t>(buf[4]) & 0b111111) << 6)
            | (static_cast<char32_t>(buf[5]) & 0b111111);
            break;
        default:
            UNREACHABLE();
            break;
    }
    loc.SetEnd(line_, column_);
    if (Peek() != '\'') {
        return Token(Token::kError, {0, 0});
    }
    MoveNext();
    return Token(Token::kCharVal, loc).With(val);
}

Token Lexer::MatchString(int quote, bool escape, bool block) {
    SourcePosition loc{line_, column_};
    MoveNext();
    
    std::string buf;
    for (;;) {
        int ch = Peek();
        if (ch == quote) {
            loc.SetEnd(line_, column_);
            MoveNext();
            const String *asts = String::New(arena_, buf.data(), buf.size());
            return Token(Token::kStringLine, loc).With(asts);
        } else if (ch == '$') {
            if (escape) {
                loc.SetEnd(line_, column_);
                in_string_template_.push(kSimpleTemplate);
                const String *asts = String::New(arena_, buf.data(), buf.size());
                return Token(Token::kStringTempletePrefix, loc).With(asts);
            }
            MoveNext();
            buf.append(1, ch);
        } else if (ch == '\\') { // Escape
            if (escape) {
                if (!MatchEscapeCharacter(&buf)) {
                    return Token(Token::kError, {line_, column_});
                }
            } else {
                MoveNext();
                buf.append(1, ch);
            }
        } else if (ch == '\n' || ch == '\r') {
            if (!block) {
                SourcePosition loc{line_, column_};
                error_feedback_->Printf(loc, "Unexpected string term, expected '\\r' '\\n'");
                return Token(Token::kError, {line_, column_});
            } else {
                MoveNext();
                line_++;
                column_ = 1;
                buf.append(1, ch);
            }
        } else if (ch == 0) {
            loc.SetEnd(line_, column_);
            error_feedback_->Printf(loc, "Unexpected string term, expected EOF");
            return Token(Token::kError, loc);
        } else {
            if (!MatchUtf8Character(&buf)) {
                return Token(Token::kError, {line_, column_});
            }
        }
    }
}

bool Lexer::MatchUtf8Character(std::string *buf) {
    int ch = Peek();
    int remain = IsUtf8Prefix(ch) - 1;
    if (remain < 0) {
        error_feedback_->Printf({line_, column_}, "Incorrect UTF8 character: '%x'", ch);
        return false;
    }
    MoveNext();
    buf->append(1, ch);
    for (int i = 0; i < remain; i++) {
        ch = Peek();
        if (ch == 0) {
            error_feedback_->Printf({line_, column_}, "Unexpected EOF");
            return false;
        } else if (ch == -1) {
            error_feedback_->Printf({line_, column_}, "I/O Error");
            return false;
        }

        if ((ch & 0xc0) != 0x80) {
            error_feedback_->Printf({line_, column_}, "Incorrect UTF8 character: '%x'", ch);
            return false;
        }
        MoveNext();
        buf->append(1, ch);
    }
    return true;
}

Token Lexer::OneCharacterError(int line, int row, const char *fmt, ...) {
    SourcePosition loc{line_, column_};
    va_list ap;
    va_start(ap, fmt);
    std::string message(base::Vsprintf(fmt, ap));
    va_end(ap);
    error_feedback_->DidFeedback(loc, message.data(), message.size());
    return Token(Token::kError, loc);
}

bool Lexer::MatchEscapeCharacter(std::string *buf) {
    int ch = MoveNext();
    switch (ch) {
        case '$':
            MoveNext();
            buf->append(1, '$');
            break;
        case 'r':
            MoveNext();
            buf->append(1, '\r');
            break;
        case 'n':
            MoveNext();
            buf->append(1, '\n');
            break;
        case 't':
            MoveNext();
            buf->append(1, '\t');
            break;
        case 'b':
            MoveNext();
            buf->append(1, '\b');
            break;
        case 'x':
        case 'X': {
            uint8_t byte = 0;
            ch = MoveNext();
            if (!IsHexChar(ch)) {
                error_feedback_->Printf({line_, column_}, "Incorrect hex character: %c", ch);
                return false;
            }
            byte = static_cast<uint8_t>(HexCharToInt(ch)) << 4;
            ch = MoveNext();
            if (!IsHexChar(ch)) {
                error_feedback_->Printf({line_, column_}, "Incorrect hex character: %c", ch);
                return false;
            }
            byte |= static_cast<uint8_t>(HexCharToInt(ch));
            buf->append(1, byte);
            MoveNext();
        } break;
        case 'u':
        case 'U': {
            UNREACHABLE();
        } break;
        default:
            break;
    }
    return true;
}

int Lexer::Peek() {
    if (buffer_position_ < buffered_.size()) {
        return buffered_[buffer_position_];
    }
    if (available_ == 0) {
        return 0;
    }
    
    size_t wanted = std::min(static_cast<size_t>(available_), kBufferSize);
    auto rs = input_file_->Read(wanted, &buffered_, &scratch_);
    if (rs.IsEof()) {
        return 0;
    }
    if (rs.fail()) {
        error_feedback_->Printf({line_, column_}, "Read file error: %s", rs.ToString().c_str());
        return -1;
    }
    available_ -= wanted;
    buffer_position_ = 0;
    return bit_cast<int>(static_cast<uint32_t>(buffered_[buffer_position_]));
}

/*static*/ bool Lexer::IsTermChar(int ch) {
    switch (ch) {
        case 0:
        case '+':
        case '-':
        case '*':
        case '/':
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
        case '.':
        case ';':
        case ':':
        case '%':
        case '\'':
        case '\"':
        case '`':
        case '#':
        case '!':
        case '^':
        case '~':
        case '&':
        case '<':
        case '>':
        case '=':
        case ',':
        case '?':
        case '\\':
        case '|':
        case '$':
        case '@':
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            return true;
        default:
            break;
    }
    return false;
}

} // namespace cpl

} // namespace yalx
