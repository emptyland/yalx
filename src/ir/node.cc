#include "ir/node.h"
#include <string.h>

namespace yalx {

namespace ir {

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
