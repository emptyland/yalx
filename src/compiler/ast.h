#ifndef YALX_COMPILER_AST_H_
#define YALX_COMPILER_AST_H_

#include "compiler/node.h"
#include "base/checking.h"
#include "base/arena-utils.h"

namespace yalx {

namespace cpl {

class AstVisitor;
using String = base::ArenaString;

class AstNode : public Node {
public:
    AstNode(Node::Kind kind, SourcePosition source_position): Node(kind, source_position) {}
    
    virtual void Accept(AstVisitor *visitor) = 0;
}; // class AstNode


class AstVisitor {
public:
#define DEFINE_METHOD(name) virtual void Visit##name(name *node) = 0;
    DECLARE_AST_NODES(DEFINE_METHOD)
#undef DEFINE_METHOD
}; // class AstVisitor


//----------------------------------------------------------------------------------------------------------------------
// FileUnit

class FileUnit : public AstNode {
public:
    class ImportEntry : public AstNode {
    public:
        ImportEntry(String *original_package_name, String *package_path, String *alias,
                    SourcePosition source_position)
            : AstNode(Node::kMaxKinds, source_position)
            , original_package_name_(DCHECK_NOTNULL(original_package_name))
            , package_path_(DCHECK_NOTNULL(package_path))
            , alias_(alias) {}
        
        void Accept(AstVisitor *visitor) override {}
        
        DEF_PTR_PROP_RW(String, original_package_name);
        DEF_PTR_PROP_RW(String, package_path);
        DEF_PTR_PROP_RW(String, alias);
    private:
        String *original_package_name_;
        String *package_path_;
        String *alias_;
    }; // class ImportEntry
    
    
    FileUnit(base::Arena *arena, String *file_name, String *file_full_path, SourcePosition source_position)
        : AstNode(Node::kFileUnit, source_position)
        , file_name_(DCHECK_NOTNULL(file_name))
        , file_full_path_(DCHECK_NOTNULL(file_full_path))
        , imports_(arena) {}
    
    
private:
    String *file_name_;
    String *file_full_path_;
    String *package_name_ = nullptr;
    base::ArenaVector<ImportEntry *> imports_;
}; // class FileUnit

} // namespace cpl

} // namespace yalx

#endif // YALX_COMPILER_AST_H_
