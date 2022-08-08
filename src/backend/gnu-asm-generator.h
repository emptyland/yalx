#pragma once
#ifndef YALX_BACKEND_GUN_ASM_GENERATOR_H_
#define YALX_BACKEND_GUN_ASM_GENERATOR_H_

#include "base/arena-utils.h"

namespace yalx {
namespace base {
class PrintingWriter;
} // namespace base
namespace ir {
class StructureModel;
class Model;
class Module;
class Type;
} // namespace ir
namespace backend {

class ConstantsPool;
class Linkage;
class InstructionFunction;


class GnuAsmGenerator {
public:
    static constexpr const char kTextSegmentName[] = "__TEXT,__text,regular,pure_instructions";
    static constexpr const char kCStringSegmentName[] = "__TEXT,__cstring,cstring_literals";
    static constexpr const char kConstSegmentName[] = "__TEXT,__const";
    static constexpr const char kDataSegmentName[] = "__DATA,__data";
    
    GnuAsmGenerator(const base::ArenaMap<std::string_view, InstructionFunction *> &funs,
                    ir::Module *module,
                    ConstantsPool *const_pool,
                    Linkage *symbols,
                    base::PrintingWriter *printer);
    virtual ~GnuAsmGenerator();
    
    DEF_PTR_SETTER(const char, comment);
    DEF_PTR_SETTER(const char, text_p2align);
    
    void EmitAll();
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(GnuAsmGenerator);
protected:
    virtual void EmitFunction(InstructionFunction *) = 0;
    void EmitSourceFilesInfo();
    void EmitNumberConstants();
    void EmitStringConstants();
    void EmitMetadata();
    int EmitGlobalSlots(std::vector<int> *refs_offset);
    void MarkRefsInClass(const ir::StructureModel *clazz, const int offset, std::vector<int> *refs_offset);
    void EmitTypeRelocation(const ir::Type &ty, base::PrintingWriter *printer);
    
    const char *comment_ = "#";
    const char *text_p2align_ = "";
    
    const base::ArenaMap<std::string_view, InstructionFunction *> &funs_;
    ir::Module *const module_;
    ConstantsPool *const const_pool_;
    Linkage *const symbols_;
    base::PrintingWriter *const printer_;
}; // class GnuAsmGenerator

} // namespace backend
} // namespace yalx

#endif // YALX_BACKEND_GUN_ASM_GENERATOR_H_
