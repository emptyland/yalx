#include "ir/node.h"
#include "ir/metadata.h"
#include "ir/utils.h"
#include "ir/runtime.h"
#include "ir/condition.h"
#include "ir/constants.h"
#include "base/io.h"
#include "base/format.h"
#include <inttypes.h>
#include <string.h>
#include <set>

namespace yalx {

namespace ir {

namespace {

template<class T>
struct OperatorPrinting {
    void PrintTo(Operator *op, base::PrintingWriter *printer) {}
};

template <>
struct OperatorPrinting<Function *> {
    void PrintTo(Operator *op, base::PrintingWriter *printer) {
        auto fun_name = OperatorWith<Function *>::Data(op)->full_name();
        printer->Write("fun ")->Write(fun_name->ToSlice());
    }
};

template <>
struct OperatorPrinting<const Handle *> {
    void PrintTo(Operator *op, base::PrintingWriter *printer) {
        auto handle = OperatorWith<const Handle *>::Data(op);
        printer->Print("%s::%s", handle->owns()->full_name()->data(), handle->name()->data());
    }
};

template <>
struct OperatorPrinting<const String *> {
    void PrintTo(Operator *op, base::PrintingWriter *printer) {
        auto str = OperatorWith<const String *>::Data(op);
        printer->Print("\"%s\"", str->data());
    }
};

template <>
struct OperatorPrinting<const Model *> {
    void PrintTo(Operator *op, base::PrintingWriter *printer) {
        auto model = OperatorWith<const Model *>::Data(op);
        printer->Write(model->name()->ToSlice());
    }
};

template <>
struct OperatorPrinting<RuntimeId> {
    void PrintTo(Operator *op, base::PrintingWriter *printer) {
        auto id = OperatorWith<RuntimeId>::Data(op);
        printer->Write(id.ToString());
    }
};

template <>
struct OperatorPrinting<IConditionId> {
    void PrintTo(Operator *op, base::PrintingWriter *printer) {
        auto id = OperatorWith<IConditionId>::Data(op);
        printer->Write(id.ToString());
    }
};

template <>
struct OperatorPrinting<FConditionId> {
    void PrintTo(Operator *op, base::PrintingWriter *printer) {
        auto id = OperatorWith<FConditionId>::Data(op);
        printer->Write(id.ToString());
    }
};

template <>
struct OperatorPrinting<int8_t> {
    void PrintTo(Operator *op, base::PrintingWriter *printer) {
        printer->Print("%" PRId8, OperatorWith<int8_t>::Data(op));
    }
};

template <>
struct OperatorPrinting<uint8_t> {
    void PrintTo(Operator *op, base::PrintingWriter *printer) {
        printer->Print("%" PRIu8, OperatorWith<uint8_t>::Data(op));
    }
};

template <>
struct OperatorPrinting<int16_t> {
    void PrintTo(Operator *op, base::PrintingWriter *printer) {
        printer->Print("%" PRId16, OperatorWith<int16_t>::Data(op));
    }
};

template <>
struct OperatorPrinting<uint16_t> {
    void PrintTo(Operator *op, base::PrintingWriter *printer) {
        printer->Print("%" PRIu16, OperatorWith<uint16_t>::Data(op));
    }
};

template <>
struct OperatorPrinting<int32_t> {
    void PrintTo(Operator *op, base::PrintingWriter *printer) {
        printer->Print("%" PRId32, OperatorWith<int32_t>::Data(op));
    }
};

template <>
struct OperatorPrinting<uint32_t> {
    void PrintTo(Operator *op, base::PrintingWriter *printer) {
        printer->Print("%" PRIu32, OperatorWith<uint32_t>::Data(op));
    }
};

template <>
struct OperatorPrinting<int64_t> {
    void PrintTo(Operator *op, base::PrintingWriter *printer) {
        printer->Print("%" PRId64, OperatorWith<int64_t>::Data(op));
    }
};

template <>
struct OperatorPrinting<uint64_t> {
    void PrintTo(Operator *op, base::PrintingWriter *printer) {
        printer->Print("%" PRIu64, OperatorWith<uint64_t>::Data(op));
    }
};

template <>
struct OperatorPrinting<float> {
    void PrintTo(Operator *op, base::PrintingWriter *printer) {
        printer->Print("%f", OperatorWith<float>::Data(op));
    }
};

template <>
struct OperatorPrinting<double> {
    void PrintTo(Operator *op, base::PrintingWriter *printer) {
        printer->Print("%f", OperatorWith<double>::Data(op));
    }
};

} // namespace

Module::Module(base::Arena *arena, const String *name, const String *full_name, const String *path, const String *full_path)
: Node(Node::kModule)
, arena_(DCHECK_NOTNULL(arena))
, name_(DCHECK_NOTNULL(name))
, full_name_(DCHECK_NOTNULL(full_name))
, path_(DCHECK_NOTNULL(path))
, full_path_(DCHECK_NOTNULL(full_path))
, named_models_(arena)
, interfaces_(arena)
, structures_(arena)
, global_values_(arena)
, funs_(arena)
, values_(arena)
, source_position_table_(arena) {
}

InterfaceModel *Module::NewInterfaceModel(const String *name, const String *full_name) {
    DCHECK(named_models_.find(name->ToSlice()) == named_models_.end());
    auto model = new (arena()) InterfaceModel(arena(), name, full_name);
    named_models_[name->ToSlice()] = model;
    interfaces_.push_back(model);
    return model;
}

StructureModel *Module::NewClassModel(const String *name, const String *full_name, StructureModel *base_of) {
    DCHECK(named_models_.find(name->ToSlice()) == named_models_.end());
    auto clazz = new (arena()) StructureModel(arena(), name, full_name, StructureModel::kClass, this, base_of);
    named_models_[name->ToSlice()] = clazz;
    structures_.push_back(clazz);
    return clazz;
}

StructureModel *Module::NewStructModel(const String *name, const String *full_name, StructureModel *base_of) {
    DCHECK(named_models_.find(name->ToSlice()) == named_models_.end());
    auto clazz = new (arena()) StructureModel(arena(), name, full_name, StructureModel::kStruct, this, base_of);
    named_models_[name->ToSlice()] = clazz;
    structures_.push_back(clazz);
    return clazz;
}

StructureModel *Module::NewEnumModel(const String *name, const String *full_name) {
    DCHECK(named_models_.find(name->ToSlice()) == named_models_.end());
    auto clazz = new (arena()) StructureModel(arena(), name, full_name, StructureModel::kEnum, this, nullptr);
    named_models_[name->ToSlice()] = clazz;
    structures_.push_back(clazz);
    return clazz;
}

Function *Module::NewFunction(const Function::Decoration decoration, const String *name, StructureModel *owns,
                              PrototypeModel *prototype) {
    auto full_name = String::New(arena(), base::Sprintf("%s.%s", owns->full_name()->data(), name->data()));
    return new (arena_) Function(arena_, decoration, name, full_name, this, prototype);
}

Function *Module::NewFunction(const Function::Decoration decoration, const String *name, const String *full_name,
                              PrototypeModel *prototype) {
    auto fun = new (arena_) Function(arena_, decoration, name, full_name, this, prototype);
    DCHECK(global_values_.find(name->ToSlice()) == global_values_.end());
    global_values_[name->ToSlice()] = GlobalSlot{funs_size(), true};
    funs_.push_back(fun);
    return fun;
}

Function *Module::NewFunction(PrototypeModel *prototype) {
    auto random_name = String::New(arena_, base::Sprintf("$unnamed$_%d", (rand() << 4) | next_unnamed_id_++));
    auto fun = NewFunction(Function::kDefault, random_name, random_name, prototype);
    return fun;
}

Function *Module::NewStandaloneFunction(const Function::Decoration decoration, const String *name,
                                        const String *full_name, PrototypeModel *prototype) {
    return new (arena_) Function(arena_, decoration, name, full_name, this, prototype);
}

Function *Module::FindFunOrNull(std::string_view name) const {
    auto iter = global_values_.find(name);
    if (iter == global_values_.end()) {
        return nullptr;
    }
    return !iter->second.fun_or_val ? nullptr : fun(iter->second.offset);
}

Value *Module::FindValOrNull(std::string_view name) const {
    auto iter = global_values_.find(name);
    if (iter == global_values_.end()) {
        return nullptr;
    }
    return iter->second.fun_or_val ? nullptr : value(iter->second.offset);
}

void Module::PrintTo(base::PrintingWriter *printer) const {
    printer
    ->Println("module %s @%s {", name()->data(), full_name()->data())
    ->Println("source-files:");
    //source_position_table_.file_name(0);
    for (size_t i = 0; i < source_position_table().file_names_size(); i++) {
        printer->Indent(1)->Println("[%zd] %s", i, source_position_table().file_name(i)->data());
    }
    printer->Write("\n");
    
    if (!values_.empty()) {
        printer->Println("globals:");
        PrintingContext global_ctx(1);
        for (auto value : values_) {
            value->PrintTo(&global_ctx, printer);
        }
        printer->Write("\n");
    }
    
    if (!interfaces_.empty()) {
        printer->Println("interfaces:");
        for (auto iface : interfaces()) {
            iface->PrintTo(1, printer);
            printer->Write("\n");
        }
    }
    
    if (!structures_.empty()) {
        printer->Println("structures:");
        for (auto clazz : structures()) {
            clazz->PrintTo(1, printer);
            printer->Write("\n");
        }
    }
    
    printer->Println("functions:");
    for (auto fun : funs()) {
        PrintingContext fun_ctx(1);
        fun->PrintTo(&fun_ctx, printer);
        printer->Write("\n");
    }
    
    for (auto clazz : structures()) {
        for (auto method : clazz->methods()) {
            PrintingContext fun_ctx(1);
            method.fun->PrintTo(&fun_ctx, printer, clazz);
            printer->Write("\n");
        }
    }
    
    printer->Println("} // @%s", full_name()->data());
}

BasicBlock *Function::NewBlock(const String *name) {
    auto block = new (arena_) BasicBlock(arena_, name);
    if (!entry_) {
        entry_ = block;
    }
    blocks_.push_back(block);
    return block;
}

Function::Function(base::Arena *arena, const Decoration decoration, const String *name, const String *full_name,
                   Module *owns, PrototypeModel *prototype)
: Node(Node::kFunction)
, arena_(DCHECK_NOTNULL(arena))
, name_(DCHECK_NOTNULL(name))
, full_name_(DCHECK_NOTNULL(full_name))
, owns_(DCHECK_NOTNULL(owns))
, prototype_(prototype)
, decoration_(decoration)
, paramaters_(arena)
, blocks_(arena) {
}

void Function::PrintTo(PrintingContext *ctx, base::PrintingWriter *printer, Model *owns) const {
    if (owns) {
        printer->Indent(ctx->indent())->Print("fun %s::%s(", owns->name()->data(), name()->data());
    } else {
        printer->Indent(ctx->indent())->Print("fun %s(", name()->data());
    }
    
    int i = 0;
    for (auto param : paramaters()) {
        if (i > 0) {
            printer->Write(", ");
        }
        ctx->OfName(param, printer);
        printer->Write(": ");
        param->type().PrintTo(printer);
        i++;
    }
    
    printer->Print("): ");
    i = 0;
    for (auto type : prototype()->return_types()) {
        if (i++ > 0) {
            printer->Write(", ");
        }
        type.PrintTo(printer);
    }
    
    if (prototype()->vargs()) {
        printer->Write(" VARGS");
    }
    
    switch (decoration()) {
        case kNative:
            printer->Write(" NATIVE");
            break;
        case kAbstract:
            printer->Write(" ABSTRACT");
            break;
        case kOverride:
            printer->Write(" OVERRIDE");
            break;
        default:
            break;
    }
    
    if (blocks().empty()) {
        printer->Write("\n");
        return;
    }
    
    printer->Writeln(" {");
    for (auto block : blocks()) {
        block->PrintTo(ctx, printer);
    }
    printer->Indent(ctx->indent())->Println("} // %s", full_name()->data());
}

void Function::ReplaceUsers(const std::map<Value *, Value *> &values, BasicBlock *begin, BasicBlock *end) {
    std::set<Value *> incoming;
    auto iter = std::find(blocks_.begin(), blocks_.end(), begin);
    if (iter == blocks_.end()) {
        return;
    }

    for (auto [_, to] : values) { incoming.insert(to); }
    
    std::set<Value *> maybe_users;
    for (;iter != blocks_.end(); iter++) {
        for (auto instr : (*iter)->instructions()) {
            if (incoming.find(instr) == incoming.end()) {
                maybe_users.insert(instr);
            }
        }
        if (*iter == end) {
            break;
        }
    }
    
    for (auto [from, to] : values) {
        std::vector<std::tuple<int, Value *>> target;
        for (auto edge : from->users()) {
            if (maybe_users.find(edge.user) != maybe_users.end()) {
                target.push_back(std::make_tuple(edge.position, edge.user));
            }
        }
        for (auto [position, user] : target) {
            user->Replace(arena(), position, from, to);
        }
    }
}

BasicBlock::BasicBlock(base::Arena *arena, const String *name)
: Node(Node::kBasicBlock)
, arena_(DCHECK_NOTNULL(arena))
//, name_(DCHECK_NOTNULL(name))
, name_(name)
, instructions_(arena)
, phi_node_users_(arena)
, inputs_(arena)
, outputs_(arena) {
}

void BasicBlock::AddInstruction(Value *value) {
    if (value->Is(Operator::kPhi)) {
        for (int i = 0; i < value->op()->control_in(); i++) {
            PhiUser user;
            user.phi = value;
            user.dest = value->InputValue(i);
            DCHECK_NOTNULL(value->InputControl(i)->AsBasicBlock())->phi_node_users_.push_back(user);
        }
    }
    instructions_.push_back(value);
}

void BasicBlock::RemoveDeads() {
    auto iter = std::remove_if(instructions_.begin(), instructions_.end(), [](Value *node) { return node->IsDead(); });
    instructions_.erase(iter, instructions_.end());
}

void BasicBlock::RemovePhiUsersOfDeads() {
    auto iter = std::remove_if(phi_node_users_.begin(), phi_node_users_.end(),
                               [](auto user) { return user.phi->IsDead() || user.dest->IsDead(); });
    phi_node_users_.erase(iter, phi_node_users_.end());
}

void BasicBlock::PrintTo(PrintingContext *ctx, base::PrintingWriter *printer) const {
    ctx
    ->OfIndent(printer)
    ->OfName(this, printer)
    ->Writeln("");
    for (auto instr : instructions()) {
        ctx->EnterIndent();
        instr->PrintTo(ctx, printer);
        ctx->ExitIndent();
    }
}

Value *Value::NewWithInputs(base::Arena *arena, const String *name, SourcePosition source_position, Type type,
                            Operator *op, Node **inputs, size_t size) {
    auto value = New0(arena, name, source_position, type, op);
    DCHECK(size == TotalInOutputs(op));
    ::memcpy(value->io_, inputs, size * sizeof(Node *));
    for (size_t i = 0; i < size; i++) {
        if (inputs[i]->IsValue()){
            inputs[i]->AsValue()->AddUser(arena, value, static_cast<int>(i));
        }
    }
    return value;
}

Value::Value(base::Arena *arena, const String *name, SourcePosition source_position, Type type, Operator *op)
: Node(Node::kValue)
, name_(name)
, source_position_(source_position)
, type_(type)
, op_(DCHECK_NOTNULL(op))
, has_users_overflow_(0)
, users_size_(0)
, inline_users_capacity_(0)
, inline_users_(nullptr) {
    overflow_users_.next = &overflow_users_;
    overflow_users_.prev = &overflow_users_;
    
#ifdef DEBUG
    ::memset(io_, 0, TotalInOutputs(op) * sizeof(io_[0]));
#endif
}

Value::User *Value::AddUser(base::Arena *arena, Value *user, int position) {
    if (!inline_users_) {
        DCHECK(!has_users_overflow_);
        inline_users_capacity_ = 8;
        inline_users_ = arena->NewArray<User>(inline_users_capacity_);
    }
    if (auto found = FindUser(user, position); found) {
        return found;
    }
    
    if (!has_users_overflow_) {
        if (users_size_ + 1 >= inline_users_capacity_) {
            has_users_overflow_ = 1;
        }
    }
    
    if (has_users_overflow_) {
        auto node = arena->New<User>();
        node->next = node;
        node->prev = node;
        node->user = user;
        node->position = position;
        QUEUE_INSERT_TAIL(&overflow_users_, node);
        users_size_ ++;
        return node;
    } else {
        auto node = &inline_users_[users_size_++];
        node->user = user;
        node->position = position;
        //users_size_ ++;
        return node;
    }
}

Value::User *Value::FindUser(Value *user, int position) {
    for (int i = 0; i < std::min(users_size_, inline_users_capacity_); i++) {
        if (inline_users_[i].user == user && inline_users_[i].position == position) {
            return &inline_users_[i];
        }
    }
    for (auto node = overflow_users_.next; node != &overflow_users_; node = node->next) {
        if (node->user == user && node->position == position) {
            return node;
        }
    }
    return nullptr;
}

void Value::RemoveUser(User *user) {
    if (user >= inline_users_ && user < inline_users_ + inline_users_capacity_) {
        auto pos = user - inline_users_;
        if (has_users_overflow_) {
            auto node = overflow_users_.next;
            QUEUE_REMOVE(node);
            inline_users_[pos] = *node;
        } else {
            auto real_size = std::min(inline_users_capacity_, users_size_);
            // [0 1 2 3 4 5 6 7]
            // [x 1 2 3 4 5 6 7]
            // [1 2 3 4 5 6 7 #]
            //
            // [0 1 2 3 4 5 6 7]
            // [0 x 2 3 4 5 6 7]
            // [0 2 3 4 5 6 7 #]
            if (pos < real_size - 1) {
                ::memcpy(&inline_users_[pos], &inline_users_[pos + 1], (real_size - pos - 1) * sizeof(User));
            }
        }
    } else {
        QUEUE_REMOVE(user);
    }
    if (QUEUE_EMPTY(&overflow_users_)) {
        has_users_overflow_ = 0;
    }
    users_size_ --;
}

void Value::Replace(base::Arena *arena, int position, Value *from, Value *to) {
    DCHECK(position >= 0 && position < TotalInOutputs(op()));
    DCHECK(io_[position] == from);
    
    auto edge = DCHECK_NOTNULL(from->FindUser(this, position));
    from->RemoveUser(edge);
    
    io_[position] = to;
    to->AddUser(arena, this, position);
}

void Value::PrintTo(PrintingContext *ctx, base::PrintingWriter *printer) const {
    DCHECK(op()->value() >= 0);
    DCHECK(op()->value() < Operator::kMaxValues);
    
    if (type().kind() == Type::kVoid) {
        ctx->OfIndent(printer);
        printer->Write(kOpcodeNames[op()->value()]);
    } else {
        ctx
        ->OfIndent(printer)
        ->OfName(this, printer)
        ->Write(" = ")
        ->Write(kOpcodeNames[op()->value()]);
    }
    
    type().PrintTo(printer->Write(" "));
    if (op()->value_out() > 1) {
        printer->Write(" out ");
        for (int i = 1; i < op()->value_out(); i++) {
            if (i > 1) {
                printer->Write(", ");
            }
            OutputValue(i)->type().PrintTo(printer);
            OutputValue(i)->IfConstantPrintTo(ctx, printer->Write(" "));
        }
    }
    
    if (op()->value_in() > 0) {
        printer->Write(" ");
        for (int i = 0; i < op()->value_in(); i++) {
            if (i > 0) {
                printer->Write(", ");
            }
            InputValue(i)->type().PrintTo(printer);
            InputValue(i)->IfConstantPrintTo(ctx, printer->Write(" "));
        }
    }
    
    if (op()->control_in() > 0) {
        printer->Write(" in [");
        for (int i = 0; i < op()->control_in(); i++) {
            if (i > 0) {
                printer->Write(", ");
            }
            ctx->OfName(InputControl(i), printer);
        }
        printer->Write("]");
    }
    
    if (op()->control_out() > 0) {
        printer->Write(" out [");
        for (int i = 0; i < op()->control_out(); i++) {
            if (i > 0) {
                printer->Write(", ");
            }
            ctx->OfName(OutputControl(i), printer);
        }
        printer->Write("]");
    }
    
    // DECL_OPERATORS_WITH_DATA
    switch (op()->value()) {
    #define DEFINE_CASE(name, type) \
        case Operator::k##name: \
            printer->Write(" <"); \
            OperatorPrinting<type>().PrintTo(op(), printer); \
            printer->Write(">"); \
            break;
            
            DECL_OPERATORS_WITH_DATA(DEFINE_CASE)
            
    #undef DEFINE_CASE
            
        default:
            break;
    }
    
    printer->Write("\n");
}

void Value::IfConstantPrintTo(PrintingContext *ctx, base::PrintingWriter *printer) const {
    switch (op()->value()) {
    #define DEFINE_CASE(name, type) \
        case Operator::k##name: \
            OperatorPrinting<type>().PrintTo(op(), printer); \
            break;
            
        DECL_CONSTANTS_WITH_DATA(DEFINE_CASE)
            
    #undef DEFINE_CASE
            
        case Operator::kNilConstant:
            printer->Write("nil");
            break;

        default:
            ctx->OfName(this, printer);
            break;
    }
}

Value::Users::Users(Value *owns)
: has_users_overflow_(owns->has_users_overflow_)
, users_size_(owns->users_size_)
, inline_users_capacity_(owns->inline_users_capacity_)
, inline_users_(owns->inline_users_)
, overflow_users_(&owns->overflow_users_) {

}

} // namespace ir

} // namespace yalx
