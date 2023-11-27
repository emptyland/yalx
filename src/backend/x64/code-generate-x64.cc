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
        case MachineRepresentation::kPointer:
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
        printer()->Writeln(".cfi_startproc");
        
        position_ = 0;
        for (auto ib : fun_->blocks()) {
            printer()->Println("Lblk%d:", ib->label());
            for (auto instr : ib->instructions()) {
                EmitParallelMove(instr->parallel_move(Instruction::kStart));
                Emit(instr);
                EmitParallelMove(instr->parallel_move(Instruction::kEnd));
                position_++;
            }
        }
        
        printer()->Writeln(".cfi_endproc");
    }

private:
    void Emit(Instruction *instr);
    void EmitParallelMove(const ParallelMove *moving);
    void EmitMove(InstructionOperand *dest, InstructionOperand *src);
    void EmitOperand(InstructionOperand *operand, X64RelocationStyle style = kDefault);
    void EmitOperands(InstructionOperand *io, InstructionOperand *input, X64RelocationStyle style = kDefault);

    static const char *Scratch(MachineRepresentation rep) {
        auto inst = RegistersConfiguration::of_x64();
        DCHECK(inst->scratch0() >= 0);
        return RegisterName(rep, inst->scratch0());
    }
    
    base::PrintingWriter *Incoming() { return printer()->Indent(1); }
    base::PrintingWriter *printer() { return owns_->printer_; }
    Linkage *symbols() { return owns_->symbols_; }
    
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
            printer()->Writeln("int3");
            break;
            
        case ArchRet:
            printer()->Writeln("retq");
            break;
            
        case ArchCall:
            printer()->Write("callq ");
            EmitOperand(instr->InputAt(0), kIndirectly);
            printer()->Writeln();
            break;
            
        case ArchFrameEnter:
            printer()->Println("pushq %rbp");
            
            printer()->Indent(1)->Writeln(".cfi_def_cfa_offset 16");
            printer()->Indent(1)->Writeln(".cfi_offset %rbp, -16");
            printer()->Indent(1)->Writeln("movq %rsp, %rbp");
            printer()->Indent(1)->Writeln(".cfi_def_cfa_register %rbp");

            if (auto size = instr->TempAt(0)->AsImmediate()->word32_value(); size > 0) {
                printer()->Indent(1)->Println("subq $%d, %%rsp", size);
            }
            break;
            
        case ArchFrameExit: {
            int has_adjust = 0;
            if (auto size = instr->TempAt(0)->AsImmediate()->word32_value(); size > 0) {
                printer()->Println("addq $%d, %%rsp", size);
                has_adjust = 1;
            }
            printer()->Indent(has_adjust)->Writeln("popq %rbp");

            printer()->Indent(1)->Writeln("retq");
        } break;
            
        case ArchJmp:
            printer()->Write("jmp ");
            EmitOperand(instr->OutputAt(0), kIndirectly);
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
            
        case X64And32:
            printer()->Write("andl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64And:
            printer()->Write("andq ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Test:
            printer()->Write("testq ");
            EmitOperands(instr->InputAt(0), instr->InputAt(1));
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
            
        case X64Movsxbw:
            printer()->Write("movsbw ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movzxbw:
            printer()->Write("movzbw ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movsxbl:
            printer()->Write("movsbl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movzxbl:
            printer()->Write("movzbl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movsxbq:
            printer()->Write("movsbl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movzxbq:
            printer()->Write("movzbl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movsxlq:
            printer()->Write("movslq ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movsxwb:
            printer()->Write("movswb ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movsxwl:
            printer()->Write("movswl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movsxwq:
            printer()->Write("movswq ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movzxwb:
            printer()->Write("movzwb ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movzxwl:
            printer()->Write("movzwl ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Movzxwq:
            printer()->Write("movzwq ");
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
            
        case X64Lea32:
            printer()->Write("leal ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case X64Lea:
            printer()->Write("leaq ");
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
            
        case X64Cmp8:
            printer()->Write("cmpb ");
            EmitOperands(instr->InputAt(0), instr->InputAt(1));
            break;
            
        case X64Cmp16:
            printer()->Write("cmpw ");
            EmitOperands(instr->InputAt(0), instr->InputAt(1));
            break;
            
        case X64Cmp32:
            printer()->Write("cmpl ");
            EmitOperands(instr->InputAt(0), instr->InputAt(1));
            break;
            
        case X64Cmp:
            printer()->Write("cmpq ");
            EmitOperands(instr->InputAt(0), instr->InputAt(1));
            break;
            
        case X64Ja:
            printer()->Write("ja ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jae:
            printer()->Write("jae ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jb:
            printer()->Write("jb ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jbe:
            printer()->Write("jbe ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jl:
            printer()->Write("jl ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jle:
            printer()->Write("jle ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jg:
            printer()->Write("jg ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jge:
            printer()->Write("jge ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Je:
            printer()->Write("je ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jne:
            printer()->Write("jne ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jp:
            printer()->Write("jp ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jpe:
            printer()->Write("jpe ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jpo:
            printer()->Write("jpo ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jc:
            printer()->Write("jc ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jo:
            printer()->Write("jo ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Js:
            printer()->Write("js ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Jz:
            printer()->Write("jz ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
        
        case X64Jnz:
            printer()->Write("jnz ");
            EmitOperand(instr->OutputAt(0));
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
            printer()->Write("seta ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setae:
            printer()->Write("setae ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setb:
            printer()->Write("setb ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setbe:
            printer()->Write("setbe ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setl:
            printer()->Write("setl ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setle:
            printer()->Write("setle ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setg:
            printer()->Write("setg ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setge:
            printer()->Write("setge ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Sete:
            printer()->Write("sete ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setne:
            printer()->Write("setne ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setp:
            printer()->Write("setp ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setpe:
            printer()->Write("setpe ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setpo:
            printer()->Write("setpo ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setc:
            printer()->Write("setc ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Seto:
            printer()->Write("seto ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Sets:
            printer()->Write("sets ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case X64Setz:
            printer()->Write("setz ");
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
            printer()->Write("ucomiss ");
            EmitOperands(instr->InputAt(1), instr->InputAt(0));
            break;
            
        case SSEFloat64Cmp:
            printer()->Write("ucomisd ");
            EmitOperands(instr->InputAt(1), instr->InputAt(0));
            break;
            
        case SSEFloat32Add:
            printer()->Write("addss ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case SSEFloat64Add:
            printer()->Write("addsd ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        default:
            UNREACHABLE();
            break;
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
        EmitMove(pair->mutable_dest(), pair->mutable_src());
    }
}

void X64CodeGenerator::FunctionGenerator::EmitMove(InstructionOperand *dest, InstructionOperand *src) {

    switch (dest->kind()) {
        case InstructionOperand::kReloaction: {
            switch (src->kind()) {
                case InstructionOperand::kConstant: {
                    printer()->Indent(1)->Write("movq ");
                    EmitOperand(src);
                    printer()->Println(", %%%s", Scratch(MachineRepresentation::kWord64));

                    printer()->Indent(1)->Write("movq ");
                    printer()->Print("%%%s, ", Scratch(MachineRepresentation::kWord64));
                    EmitOperand(dest);
                    printer()->Writeln();
                } break;
                case InstructionOperand::kImmediate: {
                    auto in = src->AsImmediate();
                    auto instr = SelectMoveInstr(in->machine_representation());
                    printer()->Indent(1)->Print("%s ", instr);
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
                printer()->Indent(1)->Print("%s ", instr);
                EmitOperands(dest, src);
                break;
            }
            DCHECK(out->IsMemoryLocation());
            switch (src->kind()) {
                case InstructionOperand::kImmediate: { // mem <- imm
                    printer()->Indent(1)->Print("%s ", instr);
                    EmitOperands(dest, src);
                } break;
                case InstructionOperand::kAllocated: {
                    auto in = src->AsAllocated();
                    if (in->IsRegisterLocation()) { // mem <- reg
                        printer()->Indent(1)->Print("%s ", instr);
                        EmitOperands(dest, src);
                    } else { // mem <- mem
                        printer()->Indent(1)->Print("%s ", instr);
                        EmitOperand(src);
                        printer()->Println(", %%%s", Scratch(out->machine_representation()));

                        printer()->Indent(1)->Print("%s ", instr);
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
            if (opd->label()) {
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
                                   ir::Module *module,
                                   ConstantsPool *const_pool,
                                   Linkage *symbols,
                                   base::PrintingWriter *printer)
: GnuAsmGenerator(funs, module, const_pool, symbols, printer) {
    set_comment("#");
    set_text_p2align("4, 0x90");
}

X64CodeGenerator::~X64CodeGenerator() {}

void X64CodeGenerator::EmitFunction(InstructionFunction *fun) {
    FunctionGenerator gen(this, fun);
    gen.EmitAll();
    
    if (fun->native_handle()) {
        FunctionGenerator g2(this, fun->native_handle());
        g2.EmitAll();
    }
}

} // namespace backend
} // namespace yalx
