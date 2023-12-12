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

namespace yalx::backend {

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
    DCHECK(id >= 0);
    if (id == 63) {
        return "sp";
    }
    switch (rep) {
        case MachineRepresentation::kWord8:
        case MachineRepresentation::kWord16:
        case MachineRepresentation::kWord32:
            if (id == 29) {
                return "fp";
            } else if (id == 30) {
                return "lr";
            } else if (id == 31) {
                return "wzr";
            }
            return kRegisterWord32Names[id];
        case MachineRepresentation::kWord64:
            if (id == 29) {
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
        //printd("%s", fun_->symbol()->data());
        printer()->Write(".global ")->Writeln(fun_->symbol()->ToSlice());
        printer()->Write(fun_->symbol()->ToSlice())->Writeln(":");
        printer()->Writeln(".cfi_startproc");
        position_ = 0;
        for (auto ib : fun_->blocks()) {
            printer()->Println("Lblk%d:", ib->label());
            for (auto instr : ib->instructions()) {
                Emit(instr);
                position_++;
            }
        }
        printer()->Writeln(".cfi_endproc");
    }

private:
    void Emit(Instruction *instr);
    void EmitOperand(InstructionOperand *operand, RelocationStyle style = kDefault);
    void EmitOperands(InstructionOperand *opd0, InstructionOperand *opd1, RelocationStyle style = kDefault);
    void EmitOperands(InstructionOperand *opd0, InstructionOperand *opd1, InstructionOperand *opd2,
                      RelocationStyle style = kDefault);
    void EmitOperands(InstructionOperand *opd0, InstructionOperand *opd1, InstructionOperand *opd2, const char *cond);
    
    base::PrintingWriter *Incoming() { return printer()->Indent(1); }
    base::PrintingWriter *printer() { return owns_->printer_; }
    Linkage *symbols() { return owns_->symbols_; }
    
    Arm64CodeGenerator *const owns_;
    InstructionFunction *fun_;
    int position_ = 0;
}; // class Arm64CodeGenerator::FunctionGenerator

void Arm64CodeGenerator::FunctionGenerator::Emit(Instruction *instr) {
    //printer()->Indent(1); // Print a indent first
    
    switch (instr->op()) {
        case ArchNop:
            printer()->Indent(1)->Writeln("nop");
            break;
            
        case ArchDebugBreak:
        case ArchUnreachable:
            printer()->Indent(1)->Writeln("brk #0x3c");
            break;
            
        case ArchRet:
            printer()->Indent(1)->Writeln("ret");
            break;
            
        case ArchCall:
            if (instr->InputAt(0)->IsReloaction()) {
                printer()->Indent(1)->Write("bl ");
            } else {
                DCHECK(AllocatedOpdOperator::IsRegister(instr->InputAt(0)));
                printer()->Indent(1)->Write("blr ");
            }
            EmitOperand(instr->InputAt(0));
            printer()->Writeln();
            break;
            
//            sub sp, sp, #80
//            stp fp, lr, [sp, #64]
//            add fp, sp, #64
        case ArchFrameEnter:
            printer()->Indent(1)->Write("sub sp, sp, ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln();
            
            printer()->Indent(1)->Write("stp ");
            EmitOperand(instr->OutputAt(0));
            printer()->Write(", lr, ");
            EmitOperand(instr->InputAt(1));
            printer()->Writeln();
            
            printer()->Indent(1)->Write("add ");
            EmitOperand(instr->OutputAt(0));
            printer()->Println(", sp, #%d", AllocatedOpdOperator::AsLocation(instr->InputAt(1))->index());
            
            printer()->Indent(1)->Writeln(".cfi_def_cfa fp, 16");
            printer()->Indent(1)->Writeln(".cfi_offset lr, -8");
            printer()->Indent(1)->Writeln(".cfi_offset fp, -16");
        break;
            
//            ldp fp, lr, [sp, #64]
//            add sp, sp, #80
//            ret
        case ArchFrameExit:
            printer()->Indent(1)->Write("ldp ");
            EmitOperand(instr->OutputAt(0));
            printer()->Write(", lr, ");
            EmitOperand(instr->InputAt(1));
            printer()->Writeln();
            
            printer()->Indent(1)->Write("add sp, sp, ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln();
            
            printer()->Indent(1)->Writeln("ret");
            break;
            
        case ArchJmp:
            printer()->Indent(1)->Write("b ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln();
            break;
            
        case Arm64B_al:
            printer()->Indent(1)->Write("b.al ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln();
            break;
            
        case Arm64B_cc:
            printer()->Indent(1)->Write("b.cc ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln();
            break;
            
        case Arm64B_cs:
            printer()->Indent(1)->Write("b.cs ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln();
            break;
            
        case Arm64B_eq:
            printer()->Indent(1)->Write("b.eq ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln();
            break;
            
        case Arm64B_ge:
            printer()->Indent(1)->Write("b.ge ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln();
            break;
            
        case Arm64B_gt:
            printer()->Indent(1)->Write("b.gt ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln();
            break;
            
        case Arm64B_hi:
            printer()->Indent(1)->Write("b.hi ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln();
            break;
            
        case Arm64B_le:
            printer()->Indent(1)->Write("b.le ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln();
            break;
            
        case Arm64B_ls:
            printer()->Indent(1)->Write("b.ls ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln();
            break;
            
        case Arm64B_lt:
            printer()->Indent(1)->Write("b.lt ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln();
            break;
            
        case Arm64B_vs:
            printer()->Indent(1)->Write("b.vs ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln();
            break;
            
        case Arm64B_mi:
            printer()->Indent(1)->Write("b.mi ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln();
            break;
            
        case Arm64B_ne:
            printer()->Indent(1)->Write("b.ne ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln();
            break;
            
        case Arm64B_nv:
            printer()->Indent(1)->Write("b.nv ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln();
            break;
            
        case Arm64B_pl:
            printer()->Indent(1)->Write("b.pl ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln();
            break;
            
        case Arm64B_vc:
            printer()->Indent(1)->Write("b.vc ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln();
            break;
            
        case Arm64Select_al:
            printer()->Indent(1)->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "AL");
            break;
            
        case Arm64Select_cc:
            printer()->Indent(1)->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "CC");
            break;
            
        case Arm64Select_cs:
            printer()->Indent(1)->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "CS");
            break;
            
        case Arm64Select_eq:
            printer()->Indent(1)->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "EQ");
            break;
            
        case Arm64Select_ge:
            printer()->Indent(1)->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "GE");
            break;
            
        case Arm64Select_gt:
            printer()->Indent(1)->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "GT");
            break;
            
        case Arm64Select_hi:
            printer()->Indent(1)->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "HI");
            break;
            
        case Arm64Select_le:
            printer()->Indent(1)->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "LE");
            break;
            
        case Arm64Select_ls:
            printer()->Indent(1)->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "LS");
            break;
            
        case Arm64Select_lt:
            printer()->Indent(1)->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "LT");
            break;
            
        case Arm64Select_vs:
            printer()->Indent(1)->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "VS");
            break;
            
        case Arm64Select_mi:
            printer()->Indent(1)->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "MI");
            break;
            
        case Arm64Select_ne:
            printer()->Indent(1)->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "NE");
            break;
            
        case Arm64Select_nv:
            printer()->Indent(1)->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "NV");
            break;
            
        case Arm64Select_pl:
            printer()->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "PL");
            break;
            
        case Arm64Select_vc:
            printer()->Indent(1)->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "VC");
            break;

        case Arm64Ldr:
        case Arm64LdrS:
        case Arm64LdrD:
            if (DCHECK_NOTNULL(AllocatedOpdOperator::AsLocation(instr->InputAt(0)))->index() < 0) {
                printer()->Indent(1)->Write("ldur ");
            } else {
                printer()->Indent(1)->Write("ldr ");
            }
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case Arm64Ldrb:
            if (DCHECK_NOTNULL(AllocatedOpdOperator::AsLocation(instr->InputAt(0)))->index() < 0) {
                printer()->Indent(1)->Write("ldurb ");
            } else {
                printer()->Indent(1)->Write("ldrb ");
            }
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case Arm64Ldrsb:
            if (DCHECK_NOTNULL(AllocatedOpdOperator::AsLocation(instr->InputAt(0)))->index() < 0) {
                printer()->Write("ldursb ");
            } else {
                printer()->Write("ldrsb ");
            }
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case Arm64LdrW:
            if (DCHECK_NOTNULL(AllocatedOpdOperator::AsLocation(instr->InputAt(0)))->index() < 0) {
                printer()->Indent(1)->Write("ldurh ");
            } else {
                printer()->Indent(1)->Write("ldrh ");
            }
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case Arm64Ldrsw:
            if (DCHECK_NOTNULL(AllocatedOpdOperator::AsLocation(instr->InputAt(0)))->index() < 0) {
                printer()->Indent(1)->Write("ldursw ");
            } else {
                printer()->Indent(1)->Write("ldrsw ");
            }
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case Arm64Ldp:
            printer()->Indent(1)->Write("ldp ");
            EmitOperands(instr->OutputAt(0), instr->OutputAt(1), instr->InputAt(0));
            break;
            
        case Arm64Str:
        case Arm64StrS:
        case Arm64StrD:
            if (DCHECK_NOTNULL(AllocatedOpdOperator::AsLocation(instr->OutputAt(0)))->index() < 0) {
                printer()->Indent(1)->Write("stur ");
            } else {
                printer()->Indent(1)->Write("str ");
            }
            EmitOperands(instr->InputAt(0), instr->OutputAt(0));
            break;
            
        case Arm64Strb:
            if (DCHECK_NOTNULL(AllocatedOpdOperator::AsLocation(instr->OutputAt(0)))->index() < 0) {
                printer()->Indent(1)->Write("sturb ");
            } else {
                printer()->Indent(1)->Write("strb ");
            }
            EmitOperands(instr->InputAt(0), instr->OutputAt(0));
            break;
            
        case Arm64Strh:
            if (DCHECK_NOTNULL(AllocatedOpdOperator::AsLocation(instr->OutputAt(0)))->index() < 0) {
                printer()->Indent(1)->Write("sturh ");
            } else {
                printer()->Indent(1)->Write("strh ");
            }
            EmitOperands(instr->InputAt(0), instr->OutputAt(0));
            break;
            
        case Arm64StrW:
            if (DCHECK_NOTNULL(AllocatedOpdOperator::AsLocation(instr->OutputAt(0)))->index() < 0) {
                printer()->Indent(1)->Write("stur ");
            } else {
                printer()->Indent(1)->Write("str ");
            }
            EmitOperands(instr->InputAt(0), instr->OutputAt(0));
            break;
            
        case Arm64Stp:
            printer()->Indent(1)->Write("stp ");
            EmitOperands(instr->InputAt(0), instr->InputAt(1), instr->OutputAt(0));
            break;
            
        case Arm64Mov:
        case Arm64Mov32:
            printer()->Indent(1)->Write("mov ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case Arm64FMov:
            printer()->Indent(1)->Write("fmov ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case Arm64Adr:
            printer()->Indent(1)->Write("adr ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
        
        case Arm64Adrp:
            printer()->Indent(1)->Write("adrp ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), kPage);
            break;
            
        case Arm64AddOff:
            printer()->Indent(1)->Write("add ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), kPageOff);
            break;
            
        case Arm64Cmp32:
        case Arm64Cmp:
            printer()->Indent(1)->Write("cmp ");
            EmitOperands(instr->InputAt(0), instr->InputAt(1));
            break;
            
        case Arm64Float32Cmp:
        case Arm64Float64Cmp:
            printer()->Indent(1)->Write("fcmp ");
            EmitOperands(instr->InputAt(0), instr->InputAt(1));
            break;
            
        case Arm64Add:
        case Arm64Add32:
            printer()->Indent(1)->Write("add ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1));
            break;
            
        case Arm64Float32Add:
            UNREACHABLE();
            break;
            
        case Arm64Sub:
            printer()->Indent(1)->Write("sub ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1));
            break;
            
        case Arm64Sub32:
            printer()->Indent(1)->Write("sub ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1));
            break;
            
        case Arm64And32:
        case Arm64And:
            printer()->Indent(1)->Write("and ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1));
            break;
            
        case Arm64Uxtb:
            printer()->Indent(1)->Write("uxtb ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case Arm64Uxth:
            printer()->Indent(1)->Write("uxth ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case Arm64Sxtb32:
            printer()->Indent(1)->Write("sxtb ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case Arm64Sxth32:
            printer()->Indent(1)->Write("sxth ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
            
        case Arm64Sxtw32:
            printer()->Indent(1)->Write("sxtw ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
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
    printer()->Writeln();
}

void Arm64CodeGenerator::FunctionGenerator::EmitOperands(InstructionOperand *opd0, InstructionOperand *opd1,
                                                         InstructionOperand *opd2, const char *cond) {
    EmitOperand(opd0, kDefault);
    printer()->Write(", ");
    EmitOperand(opd1, kDefault);
    printer()->Write(", ");
    EmitOperand(opd2, kDefault);
    printer()->Println(", %s", cond);
}

void Arm64CodeGenerator::FunctionGenerator::EmitOperands(InstructionOperand *opd0, InstructionOperand *opd1,
                                                         RelocationStyle style) {
    EmitOperand(opd0, style);
    printer()->Write(", ");
    EmitOperand(opd1, style);
    printer()->Writeln();
}


void Arm64CodeGenerator::FunctionGenerator::EmitOperand(InstructionOperand *operand, RelocationStyle style) {
    switch (operand->kind()) {
        case InstructionOperand::kAllocated: {
            auto opd = operand->AsAllocated();
            if (opd->IsRegisterLocation()) {
                auto name = RegisterName(opd->machine_representation(), opd->register_id());
                printer()->Write(name);
            } else {
                printer()->Print("[%s, #%d]", RegisterName(MachineRepresentation::kWord64,opd->register_id()),
                                 opd->index());
            }
        } break;
            
        case InstructionOperand::kConstant: {
            auto opd = operand->AsConstant();
            if (opd->type() == ConstantOperand::kString) {
                printer()->Print("Kstr.%d", opd->symbol_id());
            } else {
                DCHECK(opd->kind() == InstructionOperand::kConstant);
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
            switch (opd->machine_representation()) {
                case MachineRepresentation::kWord8:
                    printer()->Print("#%" PRId8, opd->word8_value());
                    break;
                case MachineRepresentation::kWord16:
                    printer()->Print("#%" PRId16, opd->word16_value());
                    break;
                case MachineRepresentation::kWord32:
                    printer()->Print("#%" PRId32, opd->word32_value());
                    break;
                case MachineRepresentation::kWord64:
                    printer()->Print("#%" PRId64, opd->word64_value());
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
            
        case InstructionOperand::kReloaction: {
            auto opd = operand->AsReloaction();
            if (opd->label()) {
                if (opd->offset() == 0) {
                    printer()->Print("Lblk%d", opd->label()->label());
                } else {
                    printer()->Print("Lblk%d%s%d", opd->label()->label(), opd->offset() < 0 ? "-" : "+",
                                     std::abs(opd->offset()));
                }
            } else {
                DCHECK(opd->symbol_name() != nullptr);
                if (opd->offset() == 0) {
                    printer()->Print("%s", opd->symbol_name()->data());
                } else {
                    printer()->Print("%s%s%d", opd->symbol_name()->data(), opd->offset() < 0 ? "-" : "+",
                                     std::abs(opd->offset()));
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
                                       Linkage *symbols,
                                       base::PrintingWriter *printer)
: GnuAsmGenerator(funs, module, const_pool, symbols, printer) {
    set_comment(";");
    set_text_p2align("2");
}

Arm64CodeGenerator::~Arm64CodeGenerator() = default;

void Arm64CodeGenerator::EmitFunction(InstructionFunction *fun) {
    FunctionGenerator gen(this, fun);
    gen.EmitAll();
    
    if (fun->native_handle()) {
        FunctionGenerator g2(this, fun->native_handle());
        g2.EmitAll();
    }
}

} // namespace yalx::backend
