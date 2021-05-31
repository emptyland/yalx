#include "runtime/stack.h"
#include <stdlib.h>

int yalx_init_stack(size_t size, struct stack *stack) {
    size = ROUND_UP(size, STACK_ALIGNMENT_SIZE);

    size_t total_size = size + sizeof(struct stack_core) + STACK_PADDKING_SIZE;
    void *chunk = malloc(total_size);
    if (!chunk) {
        return -1;
    }
    dbg_init_zag(chunk, total_size);
    stack->core   = (struct stack_core *)chunk;
    stack->size   = size;
    stack->top    = stack->core->frame + size;
    stack->bottom = stack->core->frame;
    return 0;
}

void yalx_free_stack(struct stack *stack) {
    size_t core_size = stack->size + sizeof(struct stack_core) + STACK_PADDKING_SIZE;
    dbg_free_zag(stack->core, core_size);
    free(stack->core);
}
