#include "ir/source-position.h"
#include "compiler/source-position.h"

namespace yalx {

namespace ir {

SourcePositionTable::Scope::Scope(std::string_view file_name,
                                  const cpl::SourcePosition &bound,
                                  SourcePositionTable *owns)
: current_line_(bound.begin_line())
, current_column_(bound.begin_column())
, owns_(owns)
, file_id_(owns->FindOrInsertFileName(file_name)) {
}

SourcePositionTable::Scope::Scope(const cpl::SourcePosition &bound, Scope *prev)
: current_line_(bound.begin_line())
, current_column_(bound.begin_column())
, owns_(prev->owns_)
, file_id_(prev->file_id()) {
}

SourcePositionTable::SourcePositionTable(base::Arena *arena)
: arena_(arena)
, file_ids_(arena)
, file_names_(arena) {
}

int SourcePositionTable::FindOrInsertFileName(std::string_view file_name) {
    auto iter = file_ids_.find(file_name);
    if (iter != file_ids_.end()) {
        return iter->second;
    }
    int spawn_id = static_cast<int>(file_names_.size());
    file_ids_[file_name] = spawn_id;
    file_names_.push_back(base::ArenaString::New(arena_, file_name));
    return spawn_id;
}

} // namespace ir

} // namespace yalx
