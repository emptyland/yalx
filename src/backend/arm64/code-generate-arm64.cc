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
            
//            sub sp, sp, #80
//            stp fp, lr, [sp, #64]
//            add fp, sp, #64
        case ArchFrameEnter:
            printer()->Write("sub sp, sp, ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            
            printer()->Indent(1)->Write("stp ");
            EmitOperand(instr->OutputAt(0));
            printer()->Write(", lr, ");
            EmitOperand(instr->InputAt(1));
            printer()->Writeln("");
            
            printer()->Indent(1)->Write("add ");
            EmitOperand(instr->OutputAt(0));
            printer()->Println(", sp, #%d", instr->InputAt(1)->AsLocation()->k());
            
            printer()->Indent(1)->Writeln(".cfi_def_cfa fp, 16");
            printer()->Indent(1)->Writeln(".cfi_offset lr, -8");
            printer()->Indent(1)->Writeln(".cfi_offset fp, -16");
        break;
            
//            ldp fp, lr, [sp, #64]
//            add sp, sp, #80
//            ret
        case ArchFrameExit:
            printer()->Write("ldp ");
            EmitOperand(instr->OutputAt(0));
            printer()->Write(", lr, ");
            EmitOperand(instr->InputAt(1));
            printer()->Writeln("");
            
            printer()->Indent(1)->Write("add sp, sp, ");
            EmitOperand(instr->InputAt(0));
            printer()->Writeln("");
            
            printer()->Indent(1)->Writeln("ret");
            break;
            
        case ArchJmp:
            printer()->Write("b ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case Arm64B_al:
            printer()->Write("b.al ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case Arm64B_cc:
            printer()->Write("b.cc ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case Arm64B_cs:
            printer()->Write("b.cs ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case Arm64B_eq:
            printer()->Write("b.eq ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case Arm64B_ge:
            printer()->Write("b.ge ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case Arm64B_gt:
            printer()->Write("b.gt ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case Arm64B_hi:
            printer()->Write("b.hi ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case Arm64B_le:
            printer()->Write("b.le ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case Arm64B_ls:
            printer()->Write("b.ls ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case Arm64B_lt:
            printer()->Write("b.lt ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case Arm64B_vs:
            printer()->Write("b.vs ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case Arm64B_mi:
            printer()->Write("b.mi ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case Arm64B_ne:
            printer()->Write("b.ne ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case Arm64B_nv:
            printer()->Write("b.nv ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case Arm64B_pl:
            printer()->Write("b.pl ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case Arm64B_vc:
            printer()->Write("b.vc ");
            EmitOperand(instr->OutputAt(0));
            printer()->Writeln("");
            break;
            
        case Arm64Select_al:
            printer()->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "AL");
            break;
            
        case Arm64Select_cc:
            printer()->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "CC");
            break;
            
        case Arm64Select_cs:
            printer()->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "CS");
            break;
            
        case Arm64Select_eq:
            printer()->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "EQ");
            break;
            
        case Arm64Select_ge:
            printer()->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "GE");
            break;
            
        case Arm64Select_gt:
            printer()->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "GT");
            break;
            
        case Arm64Select_hi:
            printer()->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "HI");
            break;
            
        case Arm64Select_le:
            printer()->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "LE");
            break;
            
        case Arm64Select_ls:
            printer()->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "LS");
            break;
            
        case Arm64Select_lt:
            printer()->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "LT");
            break;
            
        case Arm64Select_vs:
            printer()->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "VS");
            break;
            
        case Arm64Select_mi:
            printer()->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "MI");
            break;
            
        case Arm64Select_ne:
            printer()->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "NE");
            break;
            
        case Arm64Select_nv:
            printer()->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "NV");
            break;
            
        case Arm64Select_pl:
            printer()->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "PL");
            break;
            
        case Arm64Select_vc:
            printer()->Write("csel ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), "VC");
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
            
        case Arm64Ldrb:
            if (DCHECK_NOTNULL(instr->InputAt(0)->AsLocation())->k() < 0) {
                printer()->Write("ldurb ");
            } else {
                printer()->Write("ldrb ");
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
            
        case Arm64Strb:
            if (DCHECK_NOTNULL(instr->OutputAt(0)->AsLocation())->k() < 0) {
                printer()->Write("sturb ");
            } else {
                printer()->Write("strb ");
            }
            EmitOperands(instr->InputAt(0), instr->OutputAt(0));
            break;
            
        case Arm64Strh:
            if (DCHECK_NOTNULL(instr->OutputAt(0)->AsLocation())->k() < 0) {
                printer()->Write("sturh ");
            } else {
                printer()->Write("strh ");
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
            EmitOperands(instr->OutputAt(0), instr->InputAt(0));
            break;
        
        case Arm64Adrp:
            printer()->Write("adrp ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), kPage);
            break;
            
        case Arm64AddOff:
            printer()->Write("add ");
            EmitOperands(instr->OutputAt(0), instr->InputAt(0), instr->InputAt(1), kPageOff);
            break;
            
        case Arm64Cmp32:
        case Arm64Cmp:
            printer()->Write("cmp ");
            EmitOperands(instr->InputAt(0), instr->InputAt(1));
            break;
            
        case Arm64Float32Cmp:
        case Arm64Float64Cmp:
            printer()->Write("fcmp ");
            EmitOperands(instr->InputAt(0), instr->InputAt(1));
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
            if (opd->type() == ConstantOperand::kString) {
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
: GnuAsmGenerator(funs, module, const_pool, symbols, printer) {
    set_comment(";");
    set_text_p2align("2");
}

Arm64CodeGenerator::~Arm64CodeGenerator() {}

void Arm64CodeGenerator::EmitFunction(InstructionFunction *fun) {
    FunctionGenerator gen(this, fun);
    gen.EmitAll();
    
    if (fun->native_handle()) {
        FunctionGenerator g2(this, fun->native_handle());
        g2.EmitAll();
    }
}

} // namespace backend
} // namespace yalx
