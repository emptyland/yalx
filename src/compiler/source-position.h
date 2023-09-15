#pragma once
#ifndef YALX_COMPILER_SOURCE_POSITION_H_
#define YALX_COMPILER_SOURCE_POSITION_H_

#include "base/status.h"
#include "base/format.h"
#include "base/base.h"

namespace yalx::cpl {

    class SourcePosition final {
    public:
        SourcePosition(int line, int column) : SourcePosition(line, column, line, column) {}

        SourcePosition(int begin_line, int begin_column, int end_line, int end_column)
                : begin_column_(begin_column), begin_line_(begin_line), end_column_(end_column), end_line_(end_line) {}

        static SourcePosition Unknown() { return {-1, -1}; }

        DEF_VAL_GETTER(int, begin_column);

        DEF_VAL_GETTER(int, begin_line);

        DEF_VAL_GETTER(int, end_column);

        DEF_VAL_GETTER(int, end_line);

        void SetEnd(int end_line, int end_column) {
            end_column_ = end_column;
            end_line_ = end_line;
        }

        SourcePosition Concat(const SourcePosition &end) const {
            return {begin_line_, begin_column_, end.end_line(), end.end_column()};
        }

        std::string ToString() const {
            return base::Sprintf("[(%d,%d)-(%d,%d)]", begin_line(), begin_column(), end_line(), end_column());
        }

    private:
        int begin_line_ = 0;
        int begin_column_ = 0;
        int end_line_ = 0;
        int end_column_ = 0;
    }; // class SourcePosition

} // namespace yalx

#endif // YALX_COMPILER_SOURCE_POSITION_H_
