#pragma once
#ifndef YALX_RUNTIME_STACK_H_
#define YALX_RUNTIME_STACK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "runtime/runtime.h"

#define STACK_PADDKING_SIZE 512
#define STACK_RED_ZONE_SIZE 512
#define STACK_ALIGNMENT_SIZE 16

// stack:
// +---------+----------+-------------+--------+
// | padding | red_zone | frames .... | pading |
// +---------+----------+-------------+--------+
//
struct stack_core {
    u8_t padding[STACK_PADDKING_SIZE];
    u8_t red_zone[STACK_RED_ZONE_SIZE];
    u8_t frame[0];
}; // struct stack

struct stack {
    QUEUE_HEADER(struct stack);
    address_t top;
    address_t bottom;
    size_t size;
    struct stack_core *core;
}; // struct stack

int yalx_init_stack(size_t size, struct stack *stack);

void yalx_free_stack(struct stack *stack);


#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_STACK_H_
