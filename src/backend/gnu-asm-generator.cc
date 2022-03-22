#include "backend/gnu-asm-generator.h"
#include "backend/linkage-symbols.h"
#include "backend/constants-pool.h"
#include "backend/instruction.h"
#include "ir/metadata.h"
#include "ir/node.h"
#include "ir/type.h"
#include "ir/operator.h"
#include "base/io.h"
#include "runtime/object/type.h"
#include <inttypes.h>
#include <set>

namespace yalx {
namespace backend {

GnuAsmGenerator::GnuAsmGenerator(const base::ArenaMap<std::string_view, InstructionFunction *> &funs,
                                 ir::Module *module,
                                 ConstantsPool *const_pool,
                                 LinkageSymbols *symbols,
                                 base::PrintingWriter *printer)
: funs_(funs)
, module_(module)
, const_pool_(const_pool)
, symbols_(symbols)
, printer_(printer) {
    
}

GnuAsmGenerator::~GnuAsmGenerator() {}

void GnuAsmGenerator::EmitAll() {
    printer_->Println(".section %s", kTextSegmentName);
#ifdef YALX_OS_DARWIN
    printer_->Writeln(".build_version macos, 11, 0 sdk_version 12, 1");
#endif // YALX_OS_DARWIN
    printer_->Println("%s libc symbols:", comment_);
    
    EmitSourceFilesInfo();

    printer_->Println(".p2align %s", text_p2align_)->Write("\n")->Println("%s functions", comment_);
    for (auto fun : module_->funs()) {
        auto iter = funs_.find(fun->full_name()->ToSlice());
        assert(iter != funs_.end());
        EmitFunction(iter->second);
    }
    
    for (auto clazz : module_->structures()) {
        for (auto method : clazz->methods()) {
            const_pool_->FindOrInsertString(method.fun->prototype()->name());
            const_pool_->FindOrInsertString(method.fun->name());
            
            auto iter = funs_.find(method.fun->full_name()->ToSlice());
            if (iter == funs_.end()) {
                continue;
            }
            EmitFunction(iter->second);
        }
        for (auto field : clazz->fields()) {
            const_pool_->FindOrInsertString(field.name);
        }
        const_pool_->FindOrInsertString(clazz->name());
        const_pool_->FindOrInsertString(clazz->full_name());
    }
    
    // string constants:
    if (!const_pool_->string_pool().empty()) {
        printer_->Println("%s CString constants", comment_);
        printer_->Println(".section %s", kCStringSegmentName);
        for (size_t i = 0; i < const_pool_->string_pool().size(); i++) {
            auto kval = const_pool_->string_pool()[i];
            printer_->Println("Lkzs.%zd:", i);
            // TODO process unicode
            //printer_->Indent(1)->Println(".asciz \"%s\"", kval->data());
            printer_->Indent(1)->Write(".asciz \"")->EscapingWrite(kval->ToSlice())->Writeln("\"");
        }
    }
    
    if (!const_pool_->numbers().empty()) {
        printer_->Println(".section %s", kConstSegmentName);
        printer_->Writeln(".p2align 4");
        EmitNumberConstants();
    }
    
    if (!module_->structures().empty()) {
        printer_->Println(".section %s", kDataSegmentName);
        printer_->Writeln(".p2align 4");
        printer_->Println("%s classes:", comment_);
        EmitMetadata();
    }
    
    
    printer_->Println(".section %s", kDataSegmentName);
    printer_->Writeln(".p2align 4");
    
    if (!const_pool_->string_pool().empty()) {
        EmitStringConstants();
    }
}

void GnuAsmGenerator::EmitMetadata() {
//    // unique identifer
//    uint64_t id;
//    /* class tags */
//    uint8_t constraint; // enum yalx_class_constraint
//    int32_t reference_size;
//    int32_t instance_size;
//    struct yalx_class *super;
//    struct yalx_str name;
//    struct yalx_str location;
//    uint32_t n_annotations;
//    //yalx_annotation_t *annotation;
//    void *reserved0;
//    uint32_t n_fields;
//    struct yalx_class_field *fields;
//    struct yalx_class_method *ctor;
//    uint32_t n_methods;
//    struct yalx_class_method *methods;
//    uint32_t n_vtab;
//    uint32_t n_itab;
//    struct yalx_class_method **vtab;
//    struct yalx_class_method **itab;
    for (auto clazz : module_->structures()) {
        std::string buf;
        LinkageSymbols::Build(&buf, clazz->full_name()->ToSlice());
        buf.append("$class");
        
        printer_->Println(".global %s", buf.c_str());
        printer_->Println("%s:", buf.c_str());
        printer_->Indent(1)->Println(".quad %d %s id", 0/*TODO:class_id*/, comment_);
        printer_->Indent(1)->Println(".byte %d %s constraint",
                                     clazz->declaration() == ir::Model::kClass ? K_CLASS : K_STRUCT, comment_);
        printer_->Indent(1)->Println(".byte 0 %s padding", comment_);
        printer_->Indent(1)->Writeln(".byte 0");
        printer_->Indent(1)->Writeln(".byte 0");
        printer_->Indent(1)->Println(".long %d %s reference_size", clazz->ReferenceSizeInBytes(), comment_);
        printer_->Indent(1)->Println(".long %d %s instance_size", clazz->PlacementSizeInBytes(), comment_);
        printer_->Indent(1)->Println(".long 0 %s padding", comment_);
        if (clazz->base_of()) {
            std::string base_name;
            LinkageSymbols::Build(&base_name, clazz->full_name()->ToSlice());
            base_name.append("$class");
            printer_->Indent(1)->Println(".quad %s %s super", base_name.c_str() , comment_);
        } else {
            printer_->Indent(1)->Println(".quad 0 %s super" , comment_);
        }
        auto kid = const_pool_->FindOrInsertString(clazz->name());
        printer_->Indent(1)->Println(".quad Lkzs.%zd %s name", kid, comment_);
        printer_->Indent(1)->Println(".long %zd %s name", clazz->name()->size(), comment_);
        printer_->Indent(1)->Println(".long 0 %s padding", comment_);
        kid = const_pool_->FindOrInsertString(clazz->full_name());
        printer_->Indent(1)->Println(".quad Lkzs.%zd %s location", kid, comment_);
        printer_->Indent(1)->Println(".long %zd %s location", clazz->full_name()->size(), comment_);
        printer_->Indent(1)->Println(".long 0 %s padding", comment_);
        printer_->Indent(1)->Println(".long %u %s n_annotations", 0/*TODO*/, comment_);
        printer_->Indent(1)->Println(".long 0 %s padding", comment_);
        printer_->Indent(1)->Println(".quad 0 %s reserved0", comment_);
        printer_->Indent(1)->Println(".long %zd %s n_fields", clazz->fields_size(), comment_);
        printer_->Indent(1)->Println(".long 0 %s padding", comment_);
        
        buf.erase(buf.end() - 5, buf.end());
        buf.append("fields");
        printer_->Indent(1)->Println(".quad %s %s fields", clazz->fields().empty() ? "0" : buf.c_str(), comment_);
        
        buf.erase(buf.end() - 6, buf.end());
        buf.append("ctor");
        printer_->Indent(1)->Println(".quad %s %s ctor", !clazz->constructor() ? "0" : buf.c_str(), comment_);
        
        printer_->Indent(1)->Println(".long %zd %s n_methods", clazz->methods_size(), comment_);
        printer_->Indent(1)->Println(".long 0 %s padding", comment_);
        
        buf.erase(buf.end() - 4, buf.end());
        buf.append("methods");
        printer_->Indent(1)->Println(".quad %s %s methods", clazz->methods().empty() ? "0" : buf.c_str(), comment_);
        
        printer_->Indent(1)->Println(".long %zd %s n_vtab", clazz->vtab().size(), comment_);
        printer_->Indent(1)->Println(".long %zd %s n_itab", clazz->itab().size(), comment_);
        
        buf.erase(buf.end() - 7, buf.end());
        buf.append("vtab");
        printer_->Indent(1)->Println(".quad %s %s vtab", clazz->vtab().empty() ? "0" : buf.c_str(), comment_);
        
        buf.erase(buf.end() - 4, buf.end());
        buf.append("itab");
        printer_->Indent(1)->Println(".quad %s %s itab", clazz->itab().empty() ? "0" : buf.c_str(), comment_);
        
        if (!clazz->fields().empty()) {
            std::string symbol;
            LinkageSymbols::Build(&symbol, clazz->full_name()->ToSlice());
            symbol.append("$fields");
            printer_->Println("%s:", symbol.c_str());
        }
        for (auto field : clazz->fields()) {
//            uint32_t access: 2; // yalx_access_desc
//            uint32_t constraint: 2; // val? var?
//            uint32_t n_annotations;
//            void *reserved0;
//            struct yalx_str name;
//            struct yalx_class *type;
//            uint32_t offset_of_head;
            printer_->Indent(1)->Println("%s %s::%s", comment_, clazz->name()->data(), field.name->data());
            printer_->Indent(1)->Println(".long 0 %s access|constraint", comment_);
            printer_->Indent(1)->Println(".long 0 %s n_annotations", comment_);
            printer_->Indent(1)->Println(".quad 0 %s reserved0", comment_);
            auto kid = const_pool_->FindOrInsertString(field.name);
            printer_->Indent(1)->Println(".quad Lkzs.%zd %s name", kid, comment_);
            printer_->Indent(1)->Println(".long %zd %s name", field.name->size(), comment_);
            printer_->Indent(1)->Println(".long 0 %s padding", comment_);
            printer_->Indent(1)->Println(".quad 0 %s type", comment_); // TODO: set type
            printer_->Indent(1)->Println(".long %zd %s offset_of_head", field.offset, comment_);
            printer_->Indent(1)->Println(".long 0 %s padding", comment_);
        }
        
        if (!clazz->methods().empty()) {
            std::string symbol;
            LinkageSymbols::Build(&symbol, clazz->full_name()->ToSlice());
            symbol.append("$methods");
            printer_->Println("%s:", symbol.c_str());
        }
        auto method_index = 0;
        for (auto method : clazz->methods()) {
//            uint32_t index;
//            uint32_t access: 2; // yalx_access_desc
//            uint32_t is_native: 1; // is native function?
//            uint32_t is_override: 1; // is override function?
//            uint32_t should_virtual: 1; // should in virtual table?
//            uint32_t is_builtin: 1; // is builtin-function?
//            uint32_t is_ctor: 1; // is constructor?
//            uint32_t reserved: 1;
//            uint32_t vtab_index: 24;
//            uint32_t n_annotations;
//            void *reserved0;
//            struct yalx_str name;
//            struct yalx_str prototype_desc;
//            address_t entry;
            if (method.fun == clazz->constructor()) {
                std::string symbol;
                LinkageSymbols::Build(&symbol, clazz->full_name()->ToSlice());
                symbol.append("$ctor");
                printer_->Println("%s:", symbol.c_str());
            }
            printer_->Indent(1)->Println("%s %s::%s", comment_, clazz->name()->data(), method.fun->name()->data());
            printer_->Indent(1)->Println(".long %d %s index", method_index++, comment_);
            printer_->Indent(1)->Println(".long %u %s access|is_native|is_override|...", 0, comment_); // TODO:
            printer_->Indent(1)->Println(".long 0 %s n_annotations", comment_);
            printer_->Indent(1)->Println(".long 0 %s padding", comment_);
            printer_->Indent(1)->Println(".quad 0 %s reserved0", comment_);
            auto kid = const_pool_->FindOrInsertString(method.fun->name());
            printer_->Indent(1)->Println(".quad Lkzs.%zd %s name", kid, comment_);
            printer_->Indent(1)->Println(".long %zd %s name", method.fun->name()->size(), comment_);
            printer_->Indent(1)->Println(".long 0 %s padding", comment_);
            kid = const_pool_->FindOrInsertString(method.fun->prototype()->name());
            printer_->Indent(1)->Println(".quad Lkzs.%zd %s prototype_desc", kid, comment_);
            printer_->Indent(1)->Println(".long %zd %s prototype_desc", method.fun->prototype()->name()->size(), comment_);
            printer_->Indent(1)->Println(".long 0 %s padding", comment_);
            std::string symbol;
            LinkageSymbols::Build(&symbol, method.fun->full_name()->ToSlice());
            printer_->Indent(1)->Println(".quad %s %s entry", symbol.c_str(), comment_);
        }
        
        if (!clazz->vtab().empty()) {
            std::string symbol;
            LinkageSymbols::Build(&symbol, clazz->full_name()->ToSlice());
            symbol.append("$vtab");
            printer_->Println("%s:", symbol.c_str());
        }
        for (auto handle : clazz->vtab()) {
            auto method = std::get<const ir::Model::Method *>(handle->owns()->GetMember(handle));
            auto symbol = symbols_->Mangle(method->fun->full_name());
            printer_->Indent(1)->Println(".quad %s", symbol->data());
        }
        
        if (!clazz->itab().empty()) {
            std::string symbol;
            LinkageSymbols::Build(&symbol, clazz->full_name()->ToSlice());
            symbol.append("$itab");
            printer_->Println("%s:", symbol.c_str());
        }
        for (auto handle : clazz->itab()) {
            auto method = std::get<const ir::Model::Method *>(handle->owns()->GetMember(handle));
            auto symbol = symbols_->Mangle(method->fun->full_name());
            printer_->Indent(1)->Println(".quad %s", symbol->data());
        }
    }
}

void GnuAsmGenerator::EmitSourceFilesInfo() {
    for (size_t i = 0; i < module_->source_position_table().file_names_size(); i++) {
        auto file_name = module_->source_position_table().file_name(i);
        auto begin = file_name->data();
        auto end   = file_name->data() + file_name->size();
        std::string_view dir("./");
        std::string_view name = file_name->ToSlice();
        for (auto p = end - 1; p >= begin; p--) {
            if (*p == '/' || *p == '\\') {
                name = std::string_view(p + 1);
                dir  = std::string_view(begin, p - begin);
                break;
            }
        }
        printer_->Print(".file %zd ", i+1)
        ->Write("\"")->Write(dir)->Write("\"")
        ->Write(" ")
        ->Write("\"")->Write(name)->Writeln("\"");
    }
}

void GnuAsmGenerator::EmitNumberConstants() {
    printer_->Println("%s Number constants", comment_);

    // "Knnn.%zd"
    for (const auto &[slot, id] : const_pool_->numbers()) {
        printer_->Println("Knnn.%zd:", id)->Indent(1);
        switch (slot.kind) {
            case MachineRepresentation::kWord8:
                printer_->Println(".byte %" PRId8, slot.value<int8_t>());
                break;
            case MachineRepresentation::kWord16:
                printer_->Println(".short %" PRId16, slot.value<int16_t>());
                break;
            case MachineRepresentation::kWord32:
                printer_->Println(".long %" PRId32, slot.value<int16_t>());
                break;
            case MachineRepresentation::kFloat32:
                printer_->Println(".long 0x%08" PRIx32 "    %s float.%f", slot.value<uint32_t>(), comment_,
                                  slot.value<float>());
                break;
            case MachineRepresentation::kWord64:
                printer_->Println(".long %" PRId64, slot.value<int64_t>());
                break;
            case MachineRepresentation::kFloat64:
                printer_->Println(".long 0x%016" PRIx64 "    %s double.%f", slot.value<uint64_t>(), comment_,
                                  slot.value<double>());
                break;
            default:
                UNREACHABLE();
                break;
        }
    }
}

void GnuAsmGenerator::EmitStringConstants() {
    printer_->Println("%s Yalx-String constants", comment_);
    auto symbol = symbols_->Mangle(module_->full_name());
    printer_->Println(".global %s_Lksz", symbol->data());
    printer_->Println("%s_Lksz:", symbol->data());
    printer_->Indent(1)->Println(".long %zd", const_pool_->string_pool().size());
    printer_->Indent(1)->Println(".long 0 %s padding for struct lksz_header", comment_);
    for (size_t i = 0; i < const_pool_->string_pool().size(); i++) {
        printer_->Indent(1)->Println(".quad Lkzs.%zd", i);
    }
    
    printer_->Println(".global %s_Kstr", symbol->data());
    printer_->Println("%s_Kstr:", symbol->data());
    printer_->Indent(1)->Println(".long %zd", const_pool_->string_pool().size());
    printer_->Indent(1)->Println(".long 0 %s padding for struct kstr_header", comment_);
    for (size_t i = 0; i < const_pool_->string_pool().size(); i++) {
        printer_->Println("Kstr.%zd:", i);
        printer_->Indent(1)->Println(".quad 0", i);
    }
}

} // namespace backend
} // namespace yalx
