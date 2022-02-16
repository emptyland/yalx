#include "backend/x64/code-generate-x64.h"
#include "backend/linkage-symbols.h"
#include "backend/constants-pool.h"
#include "backend/instruction.h"
#include "ir/metadata.h"
#include "ir/node.h"
#include "ir/type.h"
#include "ir/operator.h"
#include "base/io.h"
#include <inttypes.h>
#include <set>


namespace yalx {
namespace backend {

// Byte Registers:
// Without REX : AL, BL, CL, DL, AH, BH, CH, DH
// With REX    : AL, BL, CL, DL, DIL, SIL, BPL, SPL, R8L - R15L
// Word Registers
// Without REX : AX, BX, CX, DX, DI, SI, BP, SP
// With REX    : AX, BX, CX, DX, DI, SI, BP, SP, R8W - R15W
// Doubleword Registers
// Without REX : EAX, EBX, ECX, EDX, EDI, ESI, EBP, ESP
// With REX    : EAX, EBX, ECX, EDX, EDI, ESI, EBP, ESP, R8D - R15D
// Quadword Registers
// Without REX : N.A.
// With REX    : RAX, RBX, RCX, RDX, RDI, RSI, RBP, RSP, R8 - R15

static const char *kRegister64Names[] = {
    "rax", "rcx", "rdx", "rbx",
    "rsp", "rbp", "rsi", "rdi",
    "r8",  "r9",  "r10", "r11",
    "r12", "r13", "r14", "r15"
};
static_assert(16 == arraysize(kRegister64Names), "fatal");
static const char *kRegister32Names[] = {
    "eax",  "ecx",  "edx",  "ebx",
    "esp",  "ebp",  "esi",  "edi",
    "r8d",  "r9d",  "r10d", "r11d",
    "r12d", "r13d", "r14d", "r15d"
};
static_assert(16 == arraysize(kRegister32Names), "fatal");
static const char *kRegister16Names[] = {
    "ax",   "cx",   "dx",   "bx",
    "sp",   "bp",   "si",   "di",
    "r8w",  "r9w",  "r10w", "r11w",
    "r12w", "r13w", "r14w", "r15w"
};
static_assert(16 == arraysize(kRegister16Names), "fatal");
static const char *kRegister8Names[] = {
    "al",   "cl",   "dl",   "bl",
    "spl",  "bpl",  "sil",  "dil",
    "r8l",  "r9l",  "r10l", "r11l",
    "r12l", "r13l", "r14l", "r15l"
};
static_assert(16 == arraysize(kRegister8Names), "fatal");

static const char *kRegisterFloatingNames[] = {
    "xmm0",   "xmm1",   "xmm2",   "xmm3",
    "xmm4",   "xmm5",   "xmm6",   "xmm7",
    "xmm8",   "xmm9",   "xmm10",  "xmm11",
    "xmm12",  "xmm13",  "xmm14",  "xmm15",
};

static const char *RegisterName(MachineRepresentation rep, int id) {
    assert(id >= 0);
    assert(id < arraysize(kRegister64Names));
    switch (rep) {
        case MachineRepresentation::kWord8:
            return kRegister8Names[id];
        case MachineRepresentation::kWord16:
            return kRegister16Names[id];
        case MachineRepresentation::kWord32:
            return kRegister32Names[id];
        case MachineRepresentation::kWord64:
            return kRegister64Names[id];
        case MachineRepresentation::kFloat32:
        case MachineRepresentation::kFloat64:
            return kRegisterFloatingNames[id];
        default:
            UNREACHABLE();
            break;
    }
}

class X64CodeGenerator::FunctionGenerator {
public:
    enum X64RelocationStyle {
        kDefault,
        kIndirectly
    };
    
    FunctionGenerator(X64CodeGenerator *owns, InstructionFunction *fun)
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
    void EmitOperand(InstructionOperand *operand, X64RelocationStyle style = kDefault);
    void EmitOperands(InstructionOperand *io, InstructionOperand *input, X64RelocationStyle style = kDefault);
    
    base::PrintingWriter *Incoming() { return printer()->Indent(1); }
    base::PrintingWriter *printer() { return owns_->printer_; }
    LinkageSymbols *symbols() { return owns_->symbols_; }
    
    X64CodeGenerator *const owns_;
    InstructionFunction *fun_;
    int position_ = 0;
}; // class X64CodeGenerator::FunctionGenerator

void X64CodeGenerator::FunctionGenerator::Emit(Instruction *instr) {
    printer()->Indent(1); // Print a indent first
    
    switch (instr->op()) {
        case ArchNop:
            printer()->Writeln("nop");
            break;
            
        case ArchDebugBreak:
        case ArchUnreachable:
            printer()->Writeln("int 3");
            break;
            
        case ArchRet:
            printer()->Writeln("retq");
            break;
            
        case ArchCall:
            printer()->Write("callq ");
            EmitOperand(instr->InputAt(0), kIndirectly);
            printer()->Writeln("");
            break;
            
        case X64Add8:
            printer()->Write("addb ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Add16:
            printer()->Write("addw ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Add32:
            printer()->Write("addl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Add:
            printer()->Write("addq ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Sub8:
            printer()->Write("subb ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Sub16:
            printer()->Write("subw ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Sub32:
            printer()->Write("subl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Sub:
            printer()->Write("subq ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;

        case X64Movb:
            printer()->Write("movb ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movw:
            printer()->Write("movw ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movl:
            printer()->Write("movl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movq:
            printer()->Write("movq ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movss:
            printer()->Write("movss ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;

        case X64Movsd:
            printer()->Write("movsd ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Push:
            printer()->Write("pushq ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Pop:
            printer()->Write("popq ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;

        default:
            UNREACHABLE();
            break;
    }
}

void X64CodeGenerator::FunctionGenerator::EmitOperands(InstructionOperand *io, InstructionOperand *input,
                                                       X64RelocationStyle style) {
    EmitOperand(input, style);
    printer()->Write(", ");
    EmitOperand(io, style);
    printer()->Write("\n");
}

void X64CodeGenerator::FunctionGenerator::EmitOperand(InstructionOperand *operand, X64RelocationStyle style) {
    switch (operand->kind()) {
        case InstructionOperand::kRegister: {
            auto opd = operand->AsRegister();
            printer()->Print("%%%s", RegisterName(opd->rep(), opd->register_id()));
        } break;
            
        case InstructionOperand::kLocation: {
            auto opd = operand->AsLocation();
            
//            V(MR)   /* [%r1            ] */      \
//            V(MRI)  /* [%r1         + K] */      \
//            V(MR1)  /* [%r1 + %r2*1    ] */      \
//            V(MR2)  /* [%r1 + %r2*2    ] */      \
//            V(MR4)  /* [%r1 + %r2*4    ] */      \
//            V(MR8)  /* [%r1 + %r2*8    ] */      \
//            V(MR1I) /* [%r1 + %r2*1 + K] */      \
//            V(MR2I) /* [%r1 + %r2*2 + K] */      \
//            V(MR4I) /* [%r1 + %r2*3 + K] */      \
//            V(MR8I) /* [%r1 + %r2*4 + K] */      \
//            V(M1)   /* [      %r2*1    ] */      \
//            V(M2)   /* [      %r2*2    ] */      \
//            V(M4)   /* [      %r2*4    ] */      \
//            V(M8)   /* [      %r2*8    ] */      \
//            V(M1I)  /* [      %r2*1 + K] */      \
//            V(M2I)  /* [      %r2*2 + K] */      \
//            V(M4I)  /* [      %r2*4 + K] */      \
//            V(M8I)  /* [      %r2*8 + K] */      \
//            V(Root) /* [%root       + K] */
            if (style == kIndirectly) {
                printer()->Write("*");
            }
            const char *r1 = opd->register0_id() < 0 ? "not-allocated"
            : RegisterName(MachineRepresentation::kWord64, opd->register0_id());
            const char *r2 = opd->register1_id() < 0 ? "not-allocated"
            : RegisterName(MachineRepresentation::kWord64, opd->register1_id());
            switch (opd->mode()) {
                case X64Mode_MR:
                    printer()->Print("(%%%s)", r1);
                    break;
                case X64Mode_MRI:
                    printer()->Print("%d(%%%s)", opd->k(), r1);
                    break;
                case X64Mode_MR1:
                    printer()->Print("(%%%s, %%%s, 1)", r1, r2);
                    break;
                case X64Mode_MR2:
                    printer()->Print("(%%%s, %%%s, 2)", r1, r2);
                    break;
                case X64Mode_MR4:
                    printer()->Print("(%%%s, %%%s, 4)", r1, r2);
                    break;
                case X64Mode_MR8:
                    printer()->Print("(%%%s, %%%s, 8)", r1, r2);
                    break;
                case X64Mode_MR1I:
                    printer()->Print("%d(%%%s, %%%s, 1)", opd->k(), r1, r2);
                    break;
                case X64Mode_MR2I:
                    printer()->Print("%d(%%%s, %%%s, 2)", opd->k(), r1, r2);
                    break;
                case X64Mode_MR4I:
                    printer()->Print("%d(%%%s, %%%s, 4)", opd->k(), r1, r2);
                    break;
                case X64Mode_MR8I:
                    printer()->Print("%d(%%%s, %%%s, 8)", opd->k(), r1, r2);
                    break;
                case X64Mode_M1:
                    printer()->Print("(%%%s, 1)", r2);
                    break;
                case X64Mode_M2:
                    printer()->Print("(%%%s, 2)", r2);
                    break;
                case X64Mode_M4:
                    printer()->Print("(%%%s, 4)", r2);
                    break;
                case X64Mode_M8:
                    printer()->Print("(%%%s, 8)", r2);
                    break;
                case X64Mode_M1I:
                    printer()->Print("%d(%%%s, 1)", opd->k(), r2);
                    break;
                case X64Mode_M2I:
                    printer()->Print("%d(%%%s, 2)", opd->k(), r2);
                    break;
                case X64Mode_M4I:
                    printer()->Print("%d(%%%s, 4)", opd->k(), r2);
                    break;
                case X64Mode_M8I:
                    printer()->Print("%d(%%%s, 8)", opd->k(), r2);
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
            
        } break;
            
        case InstructionOperand::kConstant: {
            auto opd = operand->AsConstant();
            if (opd->kind() == ConstantOperand::kString) {
                printer()->Print("Kstr.%d(%%rip)", opd->symbol_id());
            } else {
                assert(opd->kind() == ConstantOperand::kNumber);
                printer()->Print("Knnn.%d(%%rip)", opd->symbol_id());
            }
        } break;
            
        case InstructionOperand::kImmediate: {
            auto opd = operand->AsImmediate();
            switch (opd->rep()) {
                case MachineRepresentation::kWord8:
                    printer()->Print("$%" PRId8, opd->word8());
                    break;
                case MachineRepresentation::kWord16:
                    printer()->Print("$%" PRId16, opd->word16());
                    break;
                case MachineRepresentation::kWord32:
                    printer()->Print("$%" PRId32, opd->word32());
                    break;
                case MachineRepresentation::kWord64:
                    printer()->Print("$%" PRId64, opd->word64());
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
            
        case InstructionOperand::kReloaction: {
            auto opd = operand->AsReloaction();
            if (opd->label()) {
                if (style == kIndirectly) {
                    printer()->Print("*Lblk%d", opd->label()->label());
                } else {
                    printer()->Print("Lblk%d", opd->label()->label());
                }
            } else {
                assert(opd->symbol_name() != nullptr);
                if (style == kIndirectly) {
                    printer()->Print("%s", opd->symbol_name()->data());
                } else {
                    printer()->Print("%s(%%rip)", opd->symbol_name()->data());
                }
            }
        } break;

        case InstructionOperand::kInvalid:
        default:
            break;
    }
}

X64CodeGenerator::X64CodeGenerator(const base::ArenaMap<std::string_view, InstructionFunction *> &funs,
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

void X64CodeGenerator::EmitAll() {
    printer_->Writeln(".section __TEXT,__text,regular,pure_instructions");
#ifdef YALX_OS_DARWIN
    printer_->Writeln(".build_version macos, 11, 0 sdk_version 12, 1");
#endif // YALX_OS_DARWIN
    printer_->Writeln("# libc symbols:");
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
        printer_->Writeln("# External symbols:");
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
    
    printer_->Writeln(".p2align 4, 0x90")->Write("\n")->Writeln("# functions");
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
        printer_->Writeln("# CString constants");
        printer_->Writeln(".section __TEXT,__cstring,cstring_literals");
        for (size_t i = 0; i < const_pool_->string_pool().size(); i++) {
            auto kval = const_pool_->string_pool()[i];
            printer_->Println("Lkzs.%zd:", i);
            // TODO process unicode
            printer_->Indent(1)->Println(".asciz \"%s\"", kval->data());
        }
    }
    
    if (!const_pool_->numbers().empty()) {
        printer_->Writeln("# Number constants");
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
                    printer_->Println(".long 0x%08" PRIx32 "    # float.%f", slot.value<uint32_t>(),
                                      slot.value<float>());
                    break;
                case MachineRepresentation::kWord64:
                    printer_->Println(".long %" PRId64, slot.value<int64_t>());
                    break;
                case MachineRepresentation::kFloat64:
                    printer_->Println(".long 0x%016" PRIx64 "    # double.%f", slot.value<uint64_t>(),
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
        printer_->Writeln("# Yalx-String constants");
        auto symbol = symbols_->Symbolize(module_->full_name());
        printer_->Println(".global %s_Lksz", symbol->data());
        printer_->Println("%s_lksz:", symbol->data());
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
