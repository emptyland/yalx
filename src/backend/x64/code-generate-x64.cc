#include "backend/x64/code-generate-x64.h"
#include "backend/registers-configuration.h"
#include "backend/linkage-symbols.h"
#include "backend/constants-pool.h"
#include "backend/instruction.h"
#include "ir/metadata.h"
#include "ir/node.h"
#include "ir/type.h"
#include "ir/operator.h"
#include "base/io.h"
#include <inttypes.h>


namespace yalx::backend {

static const char *SelectMoveInstr(MachineRepresentation rep);

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
    DCHECK(id >= 0);
    DCHECK(id < arraysize(kRegister64Names));
    switch (rep) {
        case MachineRepresentation::kBit:
        case MachineRepresentation::kWord8:
            return kRegister8Names[id];
        case MachineRepresentation::kWord16:
            return kRegister16Names[id];
        case MachineRepresentation::kWord32:
            return kRegister32Names[id];
        case MachineRepresentation::kWord64:
        case MachineRepresentation::kPointer:
        case MachineRepresentation::kReference:
            return kRegister64Names[id];
        case MachineRepresentation::kFloat32:
        case MachineRepresentation::kFloat64:
            return kRegisterFloatingNames[id];
            // ------------------------------
        case MachineRepresentation::kNone:
            return kRegister64Names[id];
        default:
            printd("Unknown rep: %d", rep);
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
        printer()->Writeln(".cfi_startproc");
        
        position_ = 0;
        for (auto ib : fun_->blocks()) {
            printer()->Println("Lblk%d:", ib->label());
            for (auto instr : ib->instructions()) {
                EmitParallelMove(instr->parallel_move(Instruction::kStart));
                Emit(ib, instr);
                EmitParallelMove(instr->parallel_move(Instruction::kEnd));
                position_++;
            }
        }
        
        printer()->Writeln(".cfi_endproc");
    }

private:
    void Emit(InstructionBlock *ib, Instruction *instr);
    void EmitParallelMove(const ParallelMove *moving);
    void EmitMove(InstructionOperand *dest, InstructionOperand *src);
    void EmitOperand(InstructionOperand *operand, X64RelocationStyle style = kDefault);
    void EmitOperands(InstructionOperand *io, InstructionOperand *input, X64RelocationStyle style = kDefault);

    const char *Scratch(MachineRepresentation rep) {
        return RegisterName(rep, owns_->profile()->scratch0());
    }
    
    base::PrintingWriter *Incoming() { return printer()->Indent(1); }
    base::PrintingWriter *printer() { return owns_->printer_; }
    Linkage *symbols() { return owns_->symbols_; }
    
    X64CodeGenerator *const owns_;
    InstructionFunction *fun_;
    int position_ = 0;
}; // class X64CodeGenerator::FunctionGenerator

void X64CodeGenerator::FunctionGenerator::Emit(InstructionBlock *ib, Instruction *instr) {
    //printer()->Indent(1); // Print a indent first
    
    switch (instr->op()) {
        case ArchNop:
            Incoming()->Writeln("nop");
            break;
            
        case ArchDebugBreak:
        case ArchUnreachable:
            Incoming()->Writeln("int3");
            break;
            
        case ArchRet:
            Incoming()->Writeln("retq");
            break;
            
        case ArchCall:
            Incoming()->Write("callq ");
            EmitOperand(instr->TempAt(0), kIndirectly);
            printer()->Writeln();
            break;

        case ArchCallNative:
            Incoming()->Write("callq ");
            EmitOperand(instr->InputAt(0), kIndirectly);
            printer()->Writeln();
            break;

        case ArchBeforeCall: {
            for (int i = 0; i < instr->inputs_count(); i++) {
                Incoming()->Write("pushq ");
                EmitOperand(instr->InputAt(i));
                printer()->Writeln();
            }
            if (instr->temps_count() > 2 && PrepareCallHint::GetAdjustStackSize(instr) > 0) {
                Incoming()->Write("addq ");
                EmitOperand(instr->TempAt(0));
                printer()->Writeln(", %rsp");
            }
        } break;

        case ArchAfterCall:
            if (instr->temps_count() > 2 && PrepareCallHint::GetAdjustStackSize(instr) > 0) {
                Incoming()->Write("subq ");
                EmitOperand(instr->TempAt(0));
                printer()->Writeln(", %rsp");
            }
            for (int i = instr->inputs_count() - 1; i >= 0; i--) {
                Incoming()->Write("pushq ");
                EmitOperand(instr->InputAt(i));
                printer()->Writeln();
            }
            break;

        case ArchStackAlloc:
            // Ignore
            break;
            
        case ArchFrameEnter:
            Incoming()->Println("pushq %rbp");
            
            Incoming()->Writeln(".cfi_def_cfa_offset 16");
            Incoming()->Writeln(".cfi_offset %rbp, -16");
            Incoming()->Writeln("movq %rsp, %rbp");
            Incoming()->Writeln(".cfi_def_cfa_register %rbp");

            if (auto size = FrameScopeHint::GetStackMaxSize(instr); size > 0) {
                Incoming()->Println("subq $%d, %%rsp", size);
            }
            break;
            
        case ArchFrameExit: {
            if (auto size = FrameScopeHint::GetStackMaxSize(instr); size > 0) {
                Incoming()->Println("addq $%d, %%rsp", size);
            }
            Incoming()->Writeln("popq %rbp");

            Incoming()->Writeln("retq");
        } break;
            
        case ArchJmp:
            Incoming()->Write("jmp ");
            EmitOperand(instr->OutputAt(0), kIndirectly);
            printer()->Writeln("");
            break;

        case ArchStackLoad: {
            auto field_offset = instr->InputAt(1)->AsImmediate()->word32_value();
            auto slot = instr->InputAt(0)->AsAllocated();
            auto offset = slot->index() + field_offset;
            auto rep = instr->OutputAt(0)->AsAllocated()->machine_representation();
            Incoming()->Print("%s %d(%rbp), ", SelectMoveInstr(rep), offset);
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln();
        } break;
            
        case X64Add8:
            Incoming()->Write("addb ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Add16:
            Incoming()->Write("addw ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Add32:
            Incoming()->Write("addl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Add:
            Incoming()->Write("addq ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Sub8:
            Incoming()->Write("subb ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Sub16:
            Incoming()->Write("subw ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Sub32:
            Incoming()->Write("subl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Sub:
            Incoming()->Write("subq ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64And32:
            Incoming()->Write("andl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64And:
            Incoming()->Write("andq ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Test:
            Incoming()->Write("testq ");
            EmitOperands(instr->InputAt(0), instr->InputAt(1));
            break;

        case X64Movb:
            Incoming()->Write("movb ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movw:
            Incoming()->Write("movw ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movl:
            Incoming()->Write("movl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movq:
            Incoming()->Write("movq ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movsxbw:
            Incoming()->Write("movsbw ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movzxbw:
            Incoming()->Write("movzbw ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movsxbl:
            Incoming()->Write("movsbl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movzxbl:
            Incoming()->Write("movzbl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movsxbq:
            Incoming()->Write("movsbl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movzxbq:
            Incoming()->Write("movzbl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movsxlq:
            Incoming()->Write("movslq ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movsxwb:
            Incoming()->Write("movswb ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movsxwl:
            Incoming()->Write("movswl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movsxwq:
            Incoming()->Write("movswq ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movzxwb:
            Incoming()->Write("movzwb ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movzxwl:
            Incoming()->Write("movzwl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movzxwq:
            Incoming()->Write("movzwq ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movss:
            Incoming()->Write("movss ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;

        case X64Movsd:
            Incoming()->Write("movsd ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Lea32:
            Incoming()->Write("leal ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Lea:
            Incoming()->Write("leaq ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Push:
            Incoming()->Write("pushq ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Pop:
            Incoming()->Write("popq ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Cmp8:
            Incoming()->Write("cmpb ");
            EmitOperands(instr->InputAt(0), instr->InputAt(1));
            break;
            
        case X64Cmp16:
            Incoming()->Write("cmpw ");
            EmitOperands(instr->InputAt(0), instr->InputAt(1));
            break;
            
        case X64Cmp32:
            Incoming()->Write("cmpl ");
            EmitOperands(instr->InputAt(0), instr->InputAt(1));
            break;
            
        case X64Cmp:
            Incoming()->Write("cmpq ");
            EmitOperands(instr->InputAt(0), instr->InputAt(1));
            break;
            
        case X64Ja:
            Incoming()->Write("ja ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jae:
            Incoming()->Write("jae ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jb:
            Incoming()->Write("jb ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jbe:
            Incoming()->Write("jbe ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jl:
            Incoming()->Write("jl ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jle:
            Incoming()->Write("jle ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jg:
            Incoming()->Write("jg ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jge:
            Incoming()->Write("jge ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Je:
            Incoming()->Write("je ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jne:
            Incoming()->Write("jne ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jp:
            Incoming()->Write("jp ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jpe:
            Incoming()->Write("jpe ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jpo:
            Incoming()->Write("jpo ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jc:
            Incoming()->Write("jc ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jo:
            Incoming()->Write("jo ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Js:
            Incoming()->Write("js ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jz:
            Incoming()->Write("jz ");
            EmitOperand(instr->InputAt(0), kIndirectly);
            printer()->Writeln("");
            break;
        
        case X64Jnz:
            Incoming()->Write("jnz ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jna:
        case X64Jnb:
        case X64Jnc:
        case X64Jng:
        case X64Jnl:
        case X64Jno:
        case X64Jnp:
        case X64Jns:
        case X64Jnae:
        case X64Jnbe:
        case X64Jnge:
        case X64Jnle:
            UNREACHABLE();
            break;
            
        case X64Seta:
            Incoming()->Write("seta ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setae:
            Incoming()->Write("setae ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setb:
            Incoming()->Write("setb ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setbe:
            Incoming()->Write("setbe ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setl:
            Incoming()->Write("setl ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setle:
            Incoming()->Write("setle ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setg:
            Incoming()->Write("setg ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setge:
            Incoming()->Write("setge ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Sete:
            Incoming()->Write("sete ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setne:
            Incoming()->Write("setne ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setp:
            Incoming()->Write("setp ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setpe:
            Incoming()->Write("setpe ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setpo:
            Incoming()->Write("setpo ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setc:
            Incoming()->Write("setc ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Seto:
            Incoming()->Write("seto ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Sets:
            Incoming()->Write("sets ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setz:
            Incoming()->Write("setz ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setna:
        case X64Setnb:
        case X64Setnc:
        case X64Setng:
        case X64Setnl:
        case X64Setno:
        case X64Setnp:
        case X64Setns:
        case X64Setnz:
        case X64Setnae:
        case X64Setnbe:
        case X64Setnge:
        case X64Setnle:
            UNREACHABLE();
            break;


            // SSE:
            
        case SSEFloat32Cmp:
            Incoming()->Write("ucomiss ");
            EmitOperands(instr->InputAt(1), instr->InputAt(0));
            break;
            
        case SSEFloat64Cmp:
            Incoming()->Write("ucomisd ");
            EmitOperands(instr->InputAt(1), instr->InputAt(0));
            break;
            
        case SSEFloat32Add:
            Incoming()->Write("addss ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case SSEFloat64Add:
            Incoming()->Write("addsd ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        default:
            DCHECK(instr->op() < kMaxInstructionCodes);
            DCHECK(0).Hint("Unexpected instruction code: %s", kInstrCodeNames[instr->op()]);
            break;
    }

    if (instr->is_jumping_dest()) {
        printer()->Println("Jpt_%d:", ib->JumpingPosition(instr).value());
    }
}

static const char *SelectMoveInstr(MachineRepresentation rep) {
    switch (rep) {
        case MachineRepresentation::kWord8:
            return "movb";
        case MachineRepresentation::kWord16:
            return "movw";
        case MachineRepresentation::kWord32:
            return "movl";
        case MachineRepresentation::kWord64:
        case MachineRepresentation::kPointer:
        case MachineRepresentation::kReference:
            return "movq";
        case MachineRepresentation::kFloat32:
            return "movss";
        case MachineRepresentation::kFloat64:
            return "movsd";
        default:
            UNREACHABLE();
            return nullptr;
    }
}

void X64CodeGenerator::FunctionGenerator::EmitParallelMove(const ParallelMove *moving) {
    if (!moving) {
        return; // Dont need emit moving
    }

    for (auto pair : moving->moves()) {
        if (pair->should_load_address()) {
            Incoming()->Write("leaq ");
            EmitOperands(pair->mutable_dest(), pair->mutable_src());
        } else {
            EmitMove(pair->mutable_dest(), pair->mutable_src());
        }
    }
}

void X64CodeGenerator::FunctionGenerator::EmitMove(InstructionOperand *dest, InstructionOperand *src) {

    switch (dest->kind()) {
        case InstructionOperand::kReloaction: {
            switch (src->kind()) {
                case InstructionOperand::kConstant: {
                    Incoming()->Write("movq ");
                    EmitOperand(src);
                    printer()->Println(", %%%s", Scratch(MachineRepresentation::kWord64));

                    Incoming()->Write("movq ");
                    printer()->Print("%%%s, ", Scratch(MachineRepresentation::kWord64));
                    EmitOperand(dest);
                    printer()->Writeln();
                } break;
                case InstructionOperand::kImmediate: {
                    auto in = src->AsImmediate();
                    auto instr = SelectMoveInstr(in->machine_representation());
                    Incoming()->Print("%s ", instr);
                    EmitOperands(dest, src);
                } break;
                case InstructionOperand::kReloaction:
                case InstructionOperand::kAllocated:
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
        case InstructionOperand::kAllocated: {
            auto out = dest->AsAllocated();
            auto instr = SelectMoveInstr(out->machine_representation());

            if (out->IsRegisterLocation()) {
                Incoming()->Print("%s ", instr);
                EmitOperands(dest, src);
                break;
            }
            DCHECK(out->IsMemoryLocation());
            switch (src->kind()) {
                case InstructionOperand::kImmediate: { // mem <- imm
                    Incoming()->Print("%s ", instr);
                    EmitOperands(dest, src);
                } break;
                case InstructionOperand::kAllocated: {
                    auto in = src->AsAllocated();
                    if (in->IsRegisterLocation()) { // mem <- reg
                        Incoming()->Print("%s ", instr);
                        EmitOperands(dest, src);
                    } else { // mem <- mem
                        Incoming()->Print("%s ", instr);
                        EmitOperand(src);
                        printer()->Println(", %%%s", Scratch(out->machine_representation()));

                        Incoming()->Print("%s ", instr);
                        printer()->Print("%%%s, ", Scratch(out->machine_representation()));
                        EmitOperand(dest);
                        printer()->Writeln();
                    }
                } break;
                case InstructionOperand::kConstant:
                case InstructionOperand::kReloaction:
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
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
    printer()->Writeln();
}

void X64CodeGenerator::FunctionGenerator::EmitOperand(InstructionOperand *operand, X64RelocationStyle style) {
    switch (operand->kind()) {
        case InstructionOperand::kAllocated: {
            auto rep = operand->AsAllocated();
            if (rep->IsRegisterLocation()) {
                printer()->Print("%%%s", RegisterName(rep->machine_representation(), rep->register_id()));
            } else {
                DCHECK(rep->IsMemoryLocation());
                printer()->Print("%d(%%%s)", rep->index(),
                                 RegisterName(MachineRepresentation::kPointer, rep->register_id()));
            }
        } break;

#if 0
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
#endif
        case InstructionOperand::kConstant: {
            auto opd = operand->AsConstant();
            if (opd->type() == ConstantOperand::kString) {
                printer()->Print("Kstr.%d(%%rip)", opd->symbol_id());
            } else {
                DCHECK(opd->type() == ConstantOperand::kNumber);
                printer()->Print("Knnn.%d(%%rip)", opd->symbol_id());
            }
        } break;
            
        case InstructionOperand::kImmediate: {
            auto opd = operand->AsImmediate();
            switch (opd->machine_representation()) {
                case MachineRepresentation::kWord8:
                    printer()->Print("$%" PRId8, opd->word8_value());
                    break;
                case MachineRepresentation::kWord16:
                    printer()->Print("$%" PRId16, opd->word16_value());
                    break;
                case MachineRepresentation::kWord32:
                    printer()->Print("$%" PRId32, opd->word32_value());
                    break;
                case MachineRepresentation::kWord64:
                    printer()->Print("$%" PRId64, opd->word64_value());
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
            
        case InstructionOperand::kReloaction: {
            auto opd = operand->AsReloaction();
            if (opd->is_label()) {
                printer()->Print("Lblk%d", opd->label()->label());
            } else {
                assert(opd->symbol_name() != nullptr);
                if (style == kIndirectly) {
                    if (opd->offset() == 0) {
                        printer()->Print("%s", opd->symbol_name()->data());
                    } else {
                        printer()->Print("%s%s%d", opd->symbol_name()->data(),
                                         opd->offset() > 0 ? "+" : "-", std::abs(opd->offset()));
                    }
                } else {
                    if (opd->offset() == 0) {
                        printer()->Print("%s(%%rip)", opd->symbol_name()->data());
                    } else {
                        printer()->Print("%s%s%d(%%rip)", opd->symbol_name()->data(),
                                         opd->offset() > 0 ? "+" : "-", std::abs(opd->offset()));
                    }
                }
            }
        } break;

        case InstructionOperand::kInvalid:
        default:
            UNREACHABLE();
            break;
    }
}

X64CodeGenerator::X64CodeGenerator(const base::ArenaMap<std::string_view, InstructionFunction *> &funs,
                                   const RegistersConfiguration *profile,
                                   ir::Module *module,
                                   ConstantsPool *const_pool,
                                   Linkage *symbols,
                                   base::PrintingWriter *printer)
: GnuAsmGenerator(funs, profile, module, const_pool, symbols, printer) {
    set_comment("#");
    set_text_p2align("4, 0x90");
}

X64CodeGenerator::~X64CodeGenerator() = default;

void X64CodeGenerator::EmitFunction(InstructionFunction *fun) {
    FunctionGenerator gen(this, fun);
    gen.EmitAll();
    
    if (fun->native_handle()) {
        FunctionGenerator g2(this, fun->native_handle());
        g2.EmitAll();
    }
}

} // namespace yalx
