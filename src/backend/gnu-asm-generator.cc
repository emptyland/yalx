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
    
    std::string symbol;
    LinkageSymbols::Build(&symbol, module_->full_name()->ToSlice());
    symbol.append("$global_slots");
    //    struct pkg_global_slots {
    //        size_t size_in_bytes;
    //        Address slots;
    //        size_t mark_size;
    //        int marks[0];
    //    };
    
    if (!module_->values().empty()) {
        printer_->Println("%s global slots:", comment_);
        std::vector<int> marked_refs;
        auto size_in_bytes = EmitGlobalSlots(&marked_refs);
        printer_->Println(".global %s", symbol.c_str());
        printer_->Println("%s:", symbol.c_str());
        printer_->Indent(1)->Println(".quad %zd %s size_in_bytes", size_in_bytes, comment_);
        printer_->Indent(1)->Println(".quad pkg_global_slots %s slots",  comment_);
        printer_->Indent(1)->Println(".quad %zd %s mark_size", marked_refs.size(), comment_);
        //printer_->Indent(1)->Println(".space 4 %s marks", comment_);
        for (const auto off : marked_refs) {
            printer_->Indent(1)->Println(".long %d", off);
        }
    } else {
        printer_->Println(".global %s", symbol.c_str());
        printer_->Println("%s:", symbol.c_str());
        printer_->Indent(1)->Println(".quad 0 %s size_in_bytes", comment_);
        printer_->Indent(1)->Println(".quad 0 %s slots",  comment_);
        printer_->Indent(1)->Println(".quad 0 %s mark_size",  comment_);
    }
    
    if (!const_pool_->string_pool().empty()) {
        printer_->Println("%s string constants:", comment_);
        EmitStringConstants();
    }
}

int GnuAsmGenerator::EmitGlobalSlots(std::vector<int> *refs_offset) {
    printer_->Println("pkg_global_slots:");
    int offset = 0;
    
    for (auto val : module_->values()) {
        if (!val->Is(ir::Operator::kGlobalValue) &&
            !val->Is(ir::Operator::kLazyValue)) {
            continue;
        }
    
        std::string symbol;
        LinkageSymbols::Build(&symbol, val->name()->ToSlice());
        printer_->Println("%s:", symbol.c_str());

        if (val->type().IsReference()) {
            printer_->Indent(1)->Println(".quad 0");
            refs_offset->push_back(offset);
        } else {
            switch (val->type().ReferenceSizeInBytes()) {
                case 1:
                    printer_->Indent(1)->Println(".byte 0");
                    break;
                case 2:
                    printer_->Indent(1)->Println(".word 0");
                    break;
                case 4:
                    printer_->Indent(1)->Println(".long 0");
                    break;
                case 8:
                    printer_->Indent(1)->Println(".quad 0");
                    break;
                default: {
                    printer_->Indent(1)->Println(".space %zd", val->type().ReferenceSizeInBytes());
                    MarkRefsInClass(down_cast<const ir::StructureModel>(val->type().model()), offset, refs_offset);
                } break;
            }
        }
        
        auto padding_size = val->type().ReferenceSizeInBytes() % 4;
        if (padding_size > 0) {
            printer_->Indent(1)->Println(".space %zd", padding_size);
        }
        offset += RoundUp(val->type().ReferenceSizeInBytes(), 4);
    }
    return offset;
}

void GnuAsmGenerator::MarkRefsInClass(const ir::StructureModel *clazz, const int offset, std::vector<int> *refs_offset) {
    for (const auto &field : clazz->fields()) {
        if (field.type.IsReference()) {
            refs_offset->push_back(offset + static_cast<int>(field.offset));
        } else if (field.type.model() && field.type.model()->declaration() == ir::Model::kStruct) {
            MarkRefsInClass(down_cast<const ir::StructureModel>(field.type.model()),
                            offset + static_cast<int>(field.offset), refs_offset);
        }
    }
}

static int DeclarationToKind(ir::Model::Declaration decl) {
    switch (decl) {
        case ir::Model::kStruct:
            return K_STRUCT;
        case ir::Model::kClass:
            return K_CLASS;
        case ir::Model::kEnum:
            return K_ENUM;
        default:
            UNREACHABLE();
            break;
    }
    return -1;
}

static int CompactFieldCode(const ir::Model::Field &field, ir::Model::Declaration decl) {
//            uint32_t access: 2; // yalx_access_desc
//            uint32_t constraint: 2; // val? var?
//            uint32_t enum_code: 16;
    int code = 0;
    switch (field.access) {
        case ir::kPublic:
        case ir::kExport:
            code = ACC_PUBLIC;
            break;
        case ir::kPrivate:
            code = ACC_PRIVATE;
            break;
        case ir::kProtected:
            code = ACC_PROTECTED;
            break;
        default:
            break;
    }
    // TODO: constraint
    
    if (decl == ir::Model::kEnum) {
        code |= (field.enum_value << 4);
    }
    
    return code;
}

void GnuAsmGenerator::EmitMetadata() {
//    // unique identifer
//    uint64_t id;
//    /* class tags */
//    uint8_t constraint; // enum yalx_class_constraint
//    uint8_t compact_enum;
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
        printer_->Indent(1)->Println(".byte %d %s constraint", DeclarationToKind(clazz->declaration()), comment_);
        printer_->Indent(1)->Println(".byte %d %s compact enum", static_cast<int>(clazz->IsCompactEnum()), comment_);
        printer_->Indent(1)->Println(".space 2 %s padding", comment_);
        printer_->Indent(1)->Println(".long %d %s reference_size", clazz->ReferenceSizeInBytes(), comment_);
        printer_->Indent(1)->Println(".long %d %s instance_size", clazz->PlacementSizeInBytes(), comment_);
        printer_->Indent(1)->Println(".space 4 %s padding", comment_);
        if (clazz->base_of()) {
            std::string base_name;
            LinkageSymbols::Build(&base_name, clazz->base_of()->full_name()->ToSlice());
            base_name.append("$class");
            printer_->Indent(1)->Println(".quad %s %s super", base_name.c_str() , comment_);
        } else {
            printer_->Indent(1)->Println(".quad 0 %s super" , comment_);
        }
        auto kid = const_pool_->FindOrInsertString(clazz->name());
        printer_->Indent(1)->Println(".quad Lkzs.%zd %s name", kid, comment_);
        printer_->Indent(1)->Println(".long %zd %s name", clazz->name()->size(), comment_);
        printer_->Indent(1)->Println(".space 4 %s padding", comment_);
        kid = const_pool_->FindOrInsertString(clazz->full_name());
        printer_->Indent(1)->Println(".quad Lkzs.%zd %s location", kid, comment_);
        printer_->Indent(1)->Println(".long %zd %s location", clazz->full_name()->size(), comment_);
        printer_->Indent(1)->Println(".space 4 %s padding", comment_);
        printer_->Indent(1)->Println(".long %u %s n_annotations", 0/*TODO*/, comment_);
        printer_->Indent(1)->Println(".space 4 %s padding", comment_);
        printer_->Indent(1)->Println(".quad 0 %s reserved0", comment_);
        printer_->Indent(1)->Println(".long %zd %s n_fields", clazz->fields_size(), comment_);
        printer_->Indent(1)->Println(".space 4 %s padding", comment_);
        
        buf.erase(buf.end() - 5, buf.end());
        buf.append("fields");
        printer_->Indent(1)->Println(".quad %s %s fields", clazz->fields().empty() ? "0" : buf.c_str(), comment_);
        
        buf.erase(buf.end() - 6, buf.end());
        buf.append("ctor");
        printer_->Indent(1)->Println(".quad %s %s ctor", !clazz->constructor() ? "0" : buf.c_str(), comment_);
        
        printer_->Indent(1)->Println(".long %zd %s n_methods", clazz->methods_size(), comment_);
        printer_->Indent(1)->Println(".space 4 %s padding", comment_);
        
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
        
        printer_->Indent(1)->Println(".long %u %s refs_mark_len", clazz->refs_marks_size(), comment_);
        printer_->Indent(1)->Writeln(".space 4");
        for (auto mark : clazz->refs_marks()) {
            EmitTypeRelocation(mark.ty, printer_->Indent(1)->Write(".quad "));
            printer_->Indent(1)->Println(".long %zd", mark.offset);
        }

        if (!clazz->fields().empty()) {
            std::string symbol;
            LinkageSymbols::Build(&symbol, clazz->full_name()->ToSlice());
            symbol.append("$fields");
            printer_->Println("%s:", symbol.c_str());
        }
        for (auto field : clazz->fields()) {
//            uint32_t access: 2; // yalx_access_desc
//            uint32_t constraint: 2; // val? var?
//            uint32_t enum_code: 16;
//            uint32_t n_annotations;
//            void *reserved0;
//            struct yalx_str name;
//            struct yalx_class *type;
//            uint32_t offset_of_head;
            printer_->Indent(1)->Println("%s %s::%s", comment_, clazz->name()->data(), field.name->data());
            printer_->Indent(1)->Println(".long %d %s access|constraint", CompactFieldCode(field, clazz->declaration()),
                                         comment_);
            printer_->Indent(1)->Println(".long 0 %s n_annotations", comment_);
            printer_->Indent(1)->Println(".quad 0 %s reserved0", comment_);
            auto kid = const_pool_->FindOrInsertString(field.name);
            printer_->Indent(1)->Println(".quad Lkzs.%zd %s name", kid, comment_);
            printer_->Indent(1)->Println(".long %zd %s name", field.name->size(), comment_);
            printer_->Indent(1)->Println(".space 4 %s padding", comment_);
            
            //printer_->Indent(1)->Println(".quad 0 %s type", comment_); // TODO: set type
            EmitTypeRelocation(field.type, printer_->Indent(1)->Write(".quad "));
            
            printer_->Indent(1)->Println(".long %zd %s offset_of_head", field.offset, comment_);
            printer_->Indent(1)->Println(".space 4 %s padding", comment_);
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
            printer_->Indent(1)->Println(".space 4 %s padding", comment_);
            printer_->Indent(1)->Println(".quad 0 %s reserved0", comment_);
            auto kid = const_pool_->FindOrInsertString(method.fun->name());
            printer_->Indent(1)->Println(".quad Lkzs.%zd %s name", kid, comment_);
            printer_->Indent(1)->Println(".long %zd %s name", method.fun->name()->size(), comment_);
            printer_->Indent(1)->Println(".space 4 %s padding", comment_);
            kid = const_pool_->FindOrInsertString(method.fun->prototype()->name());
            printer_->Indent(1)->Println(".quad Lkzs.%zd %s prototype_desc", kid, comment_);
            printer_->Indent(1)->Println(".long %zd %s prototype_desc", method.fun->prototype()->name()->size(), comment_);
            printer_->Indent(1)->Println(".space 4 %s padding", comment_);
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

void GnuAsmGenerator::EmitTypeRelocation(const ir::Type &ty, base::PrintingWriter *printer) {
    switch (ty.kind()) {
        case ir::Type::kVoid:
            printer->Write("0");
            break;
        case ir::Type::kWord8:
        case ir::Type::kUInt8:
            printer->Print("_builtin_classes+%d", Type_u8 * sizeof(yalx_class));
            break;
        case ir::Type::kWord16:
        case ir::Type::kUInt16:
            printer->Print("_builtin_classes+%d", Type_u16 * sizeof(yalx_class));
            break;
        case ir::Type::kWord32:
        case ir::Type::kUInt32:
            printer->Print("_builtin_classes+%d", Type_u32 * sizeof(yalx_class));
            break;
        case ir::Type::kWord64:
        case ir::Type::kUInt64:
            printer->Print("_builtin_classes+%d", Type_u32 * sizeof(yalx_class));
            break;
        case ir::Type::kInt8:
            printer->Print("_builtin_classes+%d", Type_i8 * sizeof(yalx_class));
            break;
        case ir::Type::kInt16:
            printer->Print("_builtin_classes+%d", Type_i16 * sizeof(yalx_class));
            break;
        case ir::Type::kInt32:
            printer->Print("_builtin_classes+%d", Type_i32 * sizeof(yalx_class));
            break;
        case ir::Type::kInt64:
            printer->Print("_builtin_classes+%d", Type_i64 * sizeof(yalx_class));
            break;
        case ir::Type::kFloat32:
            printer->Print("_builtin_classes+%d", Type_f32 * sizeof(yalx_class));
            break;
        case ir::Type::kFloat64:
            printer->Print("_builtin_classes+%d", Type_f64 * sizeof(yalx_class));
            break;
        case ir::Type::kString:
            printer->Write("_yalx_Zplang_Zolang_ZdString$class");
            break;
        case ir::Type::kValue: {
            std::string symbol;
            LinkageSymbols::Build(&symbol, ty.model()->full_name()->ToSlice());
            symbol.append("$class");
            printer->Write(symbol);
        } break;
        case ir::Type::kReference: {
            if (ty.model()->declaration() == ir::Model::kArray) {
                auto model = down_cast<ir::ArrayModel>(ty.model());
                if (model->dimension_count() > 1) {
                    printer->Print("_builtin_classes+%d", Type_multi_dims_array * sizeof(yalx_class));
                } else {
                    printer->Print("_builtin_classes+%d", Type_array * sizeof(yalx_class));
                }
            } else if (ty.model()->declaration() == ir::Model::kChannel) {
                UNREACHABLE();
            } else {
                std::string symbol;
                LinkageSymbols::Build(&symbol, ty.model()->full_name()->ToSlice());
                symbol.append("$class");
                printer->Write(symbol);
            }
        } break;
        default:
            UNREACHABLE();
            break;
    }
    printer->Println(" %s type", comment_);
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
