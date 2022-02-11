#include "backend/x64/code-generate-x64.h"
#include "backend/constants-pool.h"
#include "backend/instruction.h"
#include "ir/metadata.h"
#include "ir/node.h"
#include "ir/type.h"
#include "ir/operator.h"
#include "base/io.h"
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
    void EmitOperand(InstructionOperand *operand);
    void EmitOperands(InstructionOperand *io, InstructionOperand *input);
    
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
            
        default:
            UNREACHABLE();
            break;
    }
}

void X64CodeGenerator::FunctionGenerator::EmitOperands(InstructionOperand *io, InstructionOperand *input) {
    EmitOperand(input);
    printer()->Write(", ");
    EmitOperand(io);
    printer()->Write("\n");
}

void X64CodeGenerator::FunctionGenerator::EmitOperand(InstructionOperand *operand) {
    switch (operand->kind()) {
        case InstructionOperand::kRegister: {
            auto opd = operand->AsRegister();
            printer()->Print("%%%s", RegisterName(opd->rep(), opd->register_id()));
        } break;
            
        case InstructionOperand::kLocation: {
            auto opd = operand->AsLocation();
            printer()->Print("%d(%%%s)", opd->k(), RegisterName(MachineRepresentation::kWord64, opd->register0_id()));
        } break;
            
        case InstructionOperand::kConstant: {
            auto opd = operand->AsConstant();
            if (opd->kind() == ConstantOperand::kString) {
                printer()->Print("Kstr.%d(%rip)", opd->symbol_id());
            } else {
                assert(opd->kind() == ConstantOperand::kNumber);
                printer()->Print("Knnn.%d(%rip)", opd->symbol_id());
            }
        } break;
            
        case InstructionOperand::kImmediate: {
            
        } break;
            
        case InstructionOperand::kReloaction: {
            auto opd = operand->AsReloaction();
            if (opd->label()) {
                printer()->Print("Lblk%d", opd->label()->label());
            } else {
                assert(opd->symbol_name() != nullptr);
                printer()->Print("%s", opd->symbol_name()->data());
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
        printer_->Print(".file %zd ")->Write(dir)->Write(" ")->Writeln(name);
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
    
    
    // TODO: string constants:
}

} // namespace backend
} // namespace yalx
