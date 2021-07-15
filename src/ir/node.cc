#include "ir/node.h"
#include "ir/metadata.h"
#include "base/format.h"
#include <string.h>

namespace yalx {

namespace ir {

Module::Module(base::Arena *arena, const String *name, const String *path, const String *full_path)
    : Node(Node::kModule)
    , arena_(DCHECK_NOTNULL(arena))
    , name_(DCHECK_NOTNULL(name))
    , path_(DCHECK_NOTNULL(path))
    , full_path_(DCHECK_NOTNULL(full_path))
    , named_models_(arena)
    , interfaces_(arena)
    , structures_(arena)
    , named_funs_(arena)
    , unnamed_funs_(arena) {
}

InterfaceModel *Module::NewInterfaceModel(const String *name) {
    assert(named_models_.find(name->ToSlice()) == named_models_.end());
    auto model = new (arena()) InterfaceModel(arena(), name);
    named_models_[name->ToSlice()] = model;
    interfaces_.push_back(model);
    return model;
}

StructureModel *Module::NewClassModel(const String *name, StructureModel *base_of) {
    assert(named_models_.find(name->ToSlice()) == named_models_.end());
    auto clazz = new (arena()) StructureModel(arena(), name, StructureModel::kClass, this, base_of);
    named_models_[name->ToSlice()] = clazz;
    structures_.push_back(clazz);
    return clazz;
}

StructureModel *Module::NewStructModel(const String *name, StructureModel *base_of) {
    assert(named_models_.find(name->ToSlice()) == named_models_.end());
    assert(named_models_.find(name->ToSlice()) == named_models_.end());
    auto clazz = new (arena()) StructureModel(arena(), name, StructureModel::kStruct, this, base_of);
    named_models_[name->ToSlice()] = clazz;
    structures_.push_back(clazz);
    return clazz;
}

Function *Module::NewFunction(const String *name, StructureModel *owns, PrototypeModel *prototype) {
    auto full_name = String::New(arena(), base::Sprintf("%s.%s.%s", name_->data(), owns->name()->data(), name->data()));
    return NewFunction(full_name, prototype);
}

Function *Module::NewFunction(const String *name, PrototypeModel *prototype) {
    auto fun = new (arena_) Function(arena_, name, this, prototype);
    assert(named_funs_.find(name->ToSlice()) == named_funs_.end());
    named_funs_[name->ToSlice()] = fun;
    return fun;
}

Function *Module::NewFunction(PrototypeModel *prototype) {
    auto random_name = String::New(arena_, base::Sprintf("$unnamed$_%d", (rand() << 4) | next_unnamed_id_++));
    auto fun = NewFunction(random_name, prototype);
    unnamed_funs_.push_back(fun);
    return fun;
}

Function *Module::NewStandaloneFunction(const String *name, PrototypeModel *prototype) {
    return new (arena_) Function(arena_, name, this, prototype);
}

BasicBlock *Function::NewBlock(const String *name) {
    auto block = new (arena_) BasicBlock(arena_, name);
    if (!entry_) {
        entry_ = block;
    }
    blocks_.push_back(block);
    return block;
}

Function::Function(base::Arena *arena, const String *name, Module *owns, PrototypeModel *prototype)
    : Node(Node::kFunction)
    , arena_(DCHECK_NOTNULL(arena))
    , name_(DCHECK_NOTNULL(name))
    , owns_(DCHECK_NOTNULL(owns))
    , prototype_(prototype)
    , paramaters_(arena)
    , blocks_(arena) {
}

BasicBlock::BasicBlock(base::Arena *arena, const String *name)
    : Node(Node::kBasicBlock)
    , arena_(DCHECK_NOTNULL(arena))
    , name_(DCHECK_NOTNULL(name))
    , instructions_(arena)
    , inputs_(arena)
    , outputs_(arena) {
}

Value *Value::NewWithInputs(base::Arena *arena, Type type, Operator *op, Node **inputs, size_t size) {
    auto value = New0(arena, type, op);
    assert(size == TotalInOutputs(op));
    ::memcpy(value->io_, inputs, size * sizeof(Node *));
    for (size_t i = 0; i < size; i++) {
        if (inputs[i]->IsValue()){
            inputs[i]->AsValue()->AddUser(arena, value, static_cast<int>(i));
        }
    }
    return value;
}

Value::Value(base::Arena *arena, Type type, Operator *op)
    : Node(Node::kValue)
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
        assert(!has_users_overflow_);
        inline_users_capacity_ = 8;
        inline_users_ = arena->NewArray<User>(inline_users_capacity_);
    }
    if (auto found = FindUser(user, position); found) {
        return found;
    }
    
    if (!has_users_overflow_) {
        if (users_size_ + 1 >= inline_users_capacity_) {
            has_users_overflow_ = 0;
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
        users_size_ ++;
        return node;
    }
}

Value::User *Value::FindUser(Value *user, int position) {
    for (int i = 0; std::min(users_size_, inline_users_capacity_); i++) {
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

} // namespace ir

} // namespace yalx
