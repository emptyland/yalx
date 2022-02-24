#include "backend/arm64/code-generate-arm64.h"
#include "backend/linkage-symbols.h"
#include "backend/constants-pool.h"
#include "backend/instruction.h"
#include "ir/metadata.h"
#include "ir/node.h"
#include "ir/type.h"
#include "ir/operator.h"
#include "arm64/asm-arm64.h"
#include "base/io.h"
#include <inttypes.h>
#include <set>

namespace yalx {
namespace backend {

static const char *kRegisterWord64Names[] = {
#define DEFINE_REG_NAME(name) #name,
    GENERAL_REGISTERS(DEFINE_REG_NAME)
#undef  DEFINE_REG_NAME
};
static_assert(32 == arraysize(kRegisterWord64Names), "fatal");
static const char *kRegisterWord32Names[] = {
#define DEFINE_REG_NAME(name) "w"#name,
    GENERAL_REGISTER_CODE_LIST(DEFINE_REG_NAME)
#undef  DEFINE_REG_NAME
};
static_assert(32 == arraysize(kRegisterWord32Names), "fatal");
static const char *kRegisterFloat32Names[] = {
#define DEFINE_REG_NAME(name) #name,
    FLOAT_REGISTERS(DEFINE_REG_NAME)
#undef  DEFINE_REG_NAME
};
static_assert(32 == arraysize(kRegisterFloat32Names), "fatal");
static const char *kRegisterFloat64Names[] = {
#define DEFINE_REG_NAME(name) #name,
    FLOAT_REGISTERS(DEFINE_REG_NAME)
#undef  DEFINE_REG_NAME
};
static_assert(32 == arraysize(kRegisterFloat64Names), "fatal");

static const char *RegisterName(MachineRepresentation rep, int id) {
    assert(id >= 0);
    if (id == 63) {
        return "sp";
    }
    switch (rep) {
        case MachineRepresentation::kWord8:
        case MachineRepresentation::kWord16:
        case MachineRepresentation::kWord32:
            if (id == 27) {
                return "cp";
            } else if (id == 29) {
                return "fp";
            } else if (id == 30) {
                return "lr";
            } else if (id == 31) {
                return "wzr";
            }
            return kRegisterWord32Names[id];
        case MachineRepresentation::kWord64:
            if (id == 27) {
                return "cp";
            } else if (id == 29) {
                return "fp";
            } else if (id == 30) {
                return "lr";
            } else if (id == 31) {
                return "xzr";
            }
            return kRegisterWord64Names[id];
        case MachineRepresentation::kFloat32:
            return kRegisterFloat32Names[id];
        case MachineRepresentation::kFloat64:
            return kRegisterFloat64Names[id];
        default:
            UNREACHABLE();
            break;
    }
}

class Arm64CodeGenerator::FunctionGenerator {
public:
    enum RelocationStyle {
        kDefault,
        kPage,
        kPageOff,
    };
    
    FunctionGenerator(Arm64CodeGenerator *owns, InstructionFunction *fun)
    : owns_(owns)
    , fun_(fun) {
    }
    
    void EmitAll() {
        printer()->Write(".global ")->Writeln(fun_->symbol()->ToSlice());
        printer()->Write(fun_->symbol()->ToSlice())->Writeln(":");
        
        position_ = 0;
        for (auto ib : fun_->blocks()) {
            printer()->Println("Lblk%d:", ib->label());
            for (auto instr : ib->instructions()) {
                Emit(instr);
                position_++;
            }
        }
    }

private:
    void Emit(Instruction *instr);
    void EmitOperand(InstructionOperand *operand, RelocationStyle style = kDefault);
    void EmitOperands(InstructionOperand *opd0, InstructionOperand *opd1, RelocationStyle style = kDefault);
    void EmitOperands(InstructionOperand *opd0, InstructionOperand *opd1, InstructionOperand *opd2,
                      RelocationStyle style = kDefault);
    
    base::PrintingWriter *Incoming() { return printer()->Indent(1); }
    base::PrintingWriter *printer() { return owns_->printer_; }
    LinkageSymbols *symbols() { return owns_->symbols_; }
    
    Arm64CodeGenerator *const owns_;
    InstructionFunction *fun_;
    int position_ = 0;
}; // class Arm64CodeGenerator::FunctionGenerator

void Arm64CodeGenerator::FunctionGenerator::Emit(Instruction *instr) {
    printer()->Indent(1); // Print a indent first
    
    switch (instr->op()) {
        case ArchNop:
            printer()->Writeln("nop");
            break;
            
        case ArchDebugBreak:
        case ArchUnreachable:
            printer()->Writeln("brk");
            break;
            
        case ArchRet:
            printer()->Writeln("ret");
            break;
            
        case ArchCall:
            if (instr->InputAt(0)->IsReloaction()) {
                printer()->Write("bl ");
            } else {
                assert(instr->InputAt(0)->IsRegister());
                printer()->Write("blr ");
            }
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
            
        case Arm64Ldr:
        case Arm64LdrS:
        case Arm64LdrD:
            if (DCHECK_NOTNULL(instr->InputAt(0)->AsLocation())->k() < 0) {
                printer()->Write("ldur ");
            } else {
                printer()->Write("ldr ");
            }
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case Arm64Ldp:
            printer()->Write("ldp ");
            EmitOperands(instr->OutputAt(0), instr->OutputAt(1), instr->InputAt(0));
            break;
            
        case Arm64Str:
        case Arm64StrS:
        case Arm64StrD:
            if (DCHECK_NOTNULL(instr->OutputAt(0)->AsLocation())->k() < 0) {
                printer()->Write("stur ");
            } else {
                printer()->Write("str ");
            }
            EmitOperands(instr->InputAt(0), instr->OutputAt(0));
            break;
            
        case Arm64Stp:
            printer()->Write("stp ");
            EmitOperands(instr->InputAt(0), instr->InputAt(1), instr->OutputAt(0));
            break;
            
        case Arm64Mov:
            printer()->Write("mov ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case Arm64FMov:
            printer()->Write("fmov ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case Arm64Adr:
            printer()->Write("adr ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
        
        case Arm64Adrp:
            printer()->Write("adrp ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), kPage);
            break;
            
        case Arm64AddOff:
            printer()->Write("add ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), kPageOff);
            break;
            
        case Arm64Add:
        case Arm64Add32:
            printer()->Write("add ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1));
            break;
            
        case Arm64Float32Add:
            UNREACHABLE();
            break;
            
        case Arm64Sub:
            printer()->Write("sub ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1));
            break;
            
        case Arm64Sub32:
            printer()->Write("sub ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1));
            break;

        default:
            UNREACHABLE();
            break;
    }
}

void Arm64CodeGenerator::FunctionGenerator::EmitOperands(InstructionOperand *opd0, InstructionOperand *opd1,
                                                         InstructionOperand *opd2, RelocationStyle style) {
    EmitOperand(opd0, style);
    printer()->Write(", ");
    EmitOperand(opd1, style);
    printer()->Write(", ");
    EmitOperand(opd2, style);
    printer()->Writeln("");
}

void Arm64CodeGenerator::FunctionGenerator::EmitOperands(InstructionOperand *opd0, InstructionOperand *opd1,
                                                         RelocationStyle style) {
    EmitOperand(opd0, style);
    printer()->Write(", ");
    EmitOperand(opd1, style);
    printer()->Writeln("");
}


void Arm64CodeGenerator::FunctionGenerator::EmitOperand(InstructionOperand *operand, RelocationStyle style) {
    switch (operand->kind()) {
        case InstructionOperand::kRegister: {
            auto reg = operand->AsRegister();
            auto name = RegisterName(reg->rep(), reg->register_id());
            printer()->Write(name);
        } break;
            
        case InstructionOperand::kLocation: {
            // TODO:
            auto loc = operand->AsLocation();
            assert(loc->mode() == Arm64Mode_MRI);
            printer()->Print("[%s, #%d]", RegisterName(MachineRepresentation::kWord64, loc->register0_id()), loc->k());
        } break;
            
        case InstructionOperand::kConstant: {
            auto opd = operand->AsConstant();
            if (opd->kind() == ConstantOperand::kString) {
                printer()->Print("Kstr.%d", opd->symbol_id());
            } else {
                assert(opd->kind() == ConstantOperand::kNumber);
                printer()->Print("Knnn.%d", opd->symbol_id());
            }
            switch (style) {
                case kDefault:
                    break;
                case kPage:
                    printer()->Write("@PAGE");
                    break;
                case kPageOff:
                    printer()->Write("@PAGEOFF");
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
            
        case InstructionOperand::kImmediate: {
            auto opd = operand->AsImmediate();
            switch (opd->rep()) {
                case MachineRepresentation::kWord8:
                    printer()->Print("#%" PRId8, opd->word8());
                    break;
                case MachineRepresentation::kWord16:
                    printer()->Print("#%" PRId16, opd->word16());
                    break;
                case MachineRepresentation::kWord32:
                    printer()->Print("#%" PRId32, opd->word32());
                    break;
                case MachineRepresentation::kWord64:
                    printer()->Print("#%" PRId64, opd->word64());
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
            
        case InstructionOperand::kReloaction: {
            auto opd = operand->AsReloaction();
            if (opd->label()) {
                printer()->Print("Lblk%d", opd->label()->label());
            } else {
                assert(opd->symbol_name() != nullptr);
                printer()->Print("%s", opd->symbol_name()->data());
                switch (style) {
                    case kDefault:
                        break;
                    case kPage:
                        printer()->Write("@PAGE");
                        break;
                    case kPageOff:
                        printer()->Write("@PAGEOFF");
                        break;
                    default:
                        UNREACHABLE();
                        break;
                }
            }
        } break;
            
        default:
            UNREACHABLE();
            break;
    }
}


Arm64CodeGenerator::Arm64CodeGenerator(const base::ArenaMap<std::string_view, InstructionFunction *> &funs,
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

void Arm64CodeGenerator::EmitAll() {
    printer_->Writeln(".section __TEXT,__text,regular,pure_instructions");
#ifdef YALX_OS_DARWIN
    printer_->Writeln(".build_version macos, 12, 0 sdk_version 12, 1");
#endif // YALX_OS_DARWIN
    printer_->Writeln("; libc symbols:");
    printer_->Writeln(".global _memcpy,_memset");
    
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
    
    std::set<std::string_view> external_symbols;
    for (auto [name, fun] : funs_) {
        for (auto [raw, symbol] : fun->external_symbols()) {
            external_symbols.insert(symbol->ToSlice());
        }
    }
    
    if (!external_symbols.empty()) {
        printer_->Writeln("; External symbols:");
        printer_->Write(".global ");
        int i = 0;
        for (auto symbol : external_symbols) {
            if (i++ > 0) {
                printer_->Write(", ");
            }
            printer_->Write(symbol);
        }
        printer_->Write("\n");
    }
    
    printer_->Writeln(".p2align 2")->Write("\n")->Writeln("; functions");
    for (auto fun : module_->funs()) {
        auto iter = funs_.find(fun->full_name()->ToSlice());
        assert(iter != funs_.end());
        
        FunctionGenerator gen(this, iter->second);
        gen.EmitAll();
    }
    
    for (auto clazz : module_->structures()) {
        for (auto method : clazz->methods()) {
            auto iter = funs_.find(method.fun->full_name()->ToSlice());
            assert(iter != funs_.end());
            
            FunctionGenerator gen(this, iter->second);
            gen.EmitAll();
        }
    }
        
    // string constants:
    if (!const_pool_->string_pool().empty()) {
        printer_->Writeln("; CString constants");
        printer_->Writeln(".section __TEXT,__cstring,cstring_literals");
        for (size_t i = 0; i < const_pool_->string_pool().size(); i++) {
            auto kval = const_pool_->string_pool()[i];
            printer_->Println("Lkzs.%zd:", i);
            // TODO process unicode
            printer_->Indent(1)->Println(".asciz \"%s\"", kval->data());
        }
    }
    
    if (!const_pool_->numbers().empty()) {
        printer_->Writeln("; Number constants");
        printer_->Writeln(".section __TEXT,__literal8,8byte_literals");
        printer_->Writeln(".p2align 4");
        
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
                    printer_->Println(".long 0x%08" PRIx32 "    ; float.%f", slot.value<uint32_t>(),
                                      slot.value<float>());
                    break;
                case MachineRepresentation::kWord64:
                    printer_->Println(".long %" PRId64, slot.value<int64_t>());
                    break;
                case MachineRepresentation::kFloat64:
                    printer_->Println(".long 0x%016" PRIx64 "    ; double.%f", slot.value<uint64_t>(),
                                      slot.value<double>());
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        }
    }
    
    // TODO: Classes metadata
    
    
    printer_->Writeln(".section __DATA,__data");
    printer_->Writeln(".p2align 4");
    
    if (!const_pool_->string_pool().empty()) {
        printer_->Writeln("; Yalx-String constants");
        auto symbol = symbols_->Symbolize(module_->full_name());
        printer_->Println(".global %s_Lksz", symbol->data());
        printer_->Println("%s_Lksz:", symbol->data());
        printer_->Indent(1)->Println(".long %zd", const_pool_->string_pool().size());
        for (size_t i = 0; i < const_pool_->string_pool().size(); i++) {
            printer_->Indent(1)->Println(".quad Lkzs.%zd", i);
        }
        
        printer_->Println(".global %s_Kstr", symbol->data());
        printer_->Println("%s_Kstr:", symbol->data());
        printer_->Indent(1)->Println(".long %zd", const_pool_->string_pool().size());
        for (size_t i = 0; i < const_pool_->string_pool().size(); i++) {
            printer_->Println("Kstr.%zd:", i);
            printer_->Indent(1)->Println(".quad 0", i);
        }
    }
}


} // namespace backend
} // namespace yalx
