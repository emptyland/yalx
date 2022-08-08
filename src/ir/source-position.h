#pragma once
#ifndef YALX_IR_SOURCE_POSITION_H_
#define YALX_IR_SOURCE_POSITION_H_

#include "base/arena-utils.h"
#include "base/checking.h"
#include "base/base.h"
#include <string_view>

namespace yalx {
namespace cpl {
class SourcePosition;
};
namespace ir {

class SourcePosition final {
public:
    constexpr SourcePosition(int file_id, int line, int column)
        : file_id_(file_id)
        , line_(line)
        , column_(column) {}
    
    DEF_VAL_GETTER(int, file_id);
    DEF_VAL_GETTER(int, line);
    DEF_VAL_GETTER(int, column);
    
    static constexpr SourcePosition Unknown() { return {0, -1, -1}; }
    
    bool operator == (const SourcePosition &other) const {
        return file_id() == other.file_id() && line() == other.line() && column() == other.column();
    }
    
    bool operator != (const SourcePosition &other) const { return !operator==(other); }
    
private:
    const int file_id_;
    const int line_;
    const int column_;
}; // class SourcePosition


class SourcePositionTable final {
public:
    class Scope {
    public:
        Scope(std::string_view file_name, const cpl::SourcePosition &bound, SourcePositionTable *owns);
        Scope(const cpl::SourcePosition &bound, Scope *prev);
        
        DEF_VAL_GETTER(int, current_line);
        DEF_VAL_GETTER(int, current_column);
        DEF_VAL_GETTER(int, file_id);
        
        SourcePosition Position() const { return SourcePosition{file_id_, current_line_, current_column_}; }
        
        DISALLOW_IMPLICIT_CONSTRUCTORS(Scope);
    private:
        const int current_line_;
        const int current_column_;
        const int file_id_;
        SourcePositionTable *const owns_;
    }; // class Scope
    
    explicit SourcePositionTable(base::Arena *arena);
    
    DEF_ARENA_VECTOR_GETTER(const base::ArenaString *, file_name);
    
    int FindOrInsertFileName(std::string_view file_name);
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(SourcePositionTable);
private:
    base::Arena *arena_;
    base::ArenaMap<std::string_view, int> file_ids_;
    base::ArenaVector<const base::ArenaString *> file_names_;
}; // class SourcePositionTable

} // namespace ir

} // namespace yalx

#endif // YALX_IR_SOURCE_POSITION_H_
