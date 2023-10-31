#include "runtime/stack.h"
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

int yalx_init_stack(size_t size, struct stack *stack) {
    size = ROUND_UP(size, STACK_ALIGNMENT_SIZE);

    size_t total_size = size + sizeof(struct stack_core) + STACK_PADDKING_SIZE;
    void *chunk = malloc(total_size);
    if (!chunk) {
        return -1;
    }
    dbg_init_zag(chunk, total_size);
    stack->next   = stack;
    stack->prev   = stack;
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

struct stack *yalx_new_stack_from_pool(struct stack_pool *pool, size_t size) {
    //struct stack *stack =
    if (pool->nstack > 0) {
        for (struct stack *s = pool->head.next; s != &pool->head; s = s->next) {
            if (s->size <= size) {
                pool->nstack--;
                pool->used -= s->size;
                QUEUE_REMOVE(s);
                return s;
            }
        }
    }
    struct stack *s = (struct stack *)malloc(sizeof(struct stack));
    if (!s) {
        return NULL;
    }
    if (yalx_init_stack(size, s) < 0) {
        return NULL;
    }
    return s;
}

void yalx_delete_stack_to_pool(struct stack_pool *pool, struct stack *stack) {
    if (pool->used + stack->size < pool->limit) {
        QUEUE_INSERT_HEAD(&pool->head, stack);
        pool->nstack++;
        pool->used += stack->size;
        return;
    }
    
    yalx_free_stack(stack);
    free(stack);
}


struct stack *yalx_new_stack(size_t size) {
    yalx_mutex_lock(&stack_pool.mutex);
    struct stack *stack = yalx_new_stack_from_pool(&stack_pool, size);
    yalx_mutex_unlock(&stack_pool.mutex);
    return stack;
}

void yalx_delete_stack(struct stack *stack) {
    yalx_mutex_lock(&stack_pool.mutex);
    yalx_delete_stack_to_pool(&stack_pool, stack);
    yalx_mutex_unlock(&stack_pool.mutex);
}

void yalx_init_stack_pool(struct stack_pool *pool, size_t limit) {
    memset(pool, 0, sizeof(*pool));
    pool->head.next = &pool->head;
    pool->head.prev = &pool->head;
    pool->limit = limit;
    yalx_mutex_init(&pool->mutex);
}

void yalx_free_stack_pool(struct stack_pool *pool) {
    while (!QUEUE_EMPTY(&pool->head)) {
        struct stack *s = pool->head.next;
        QUEUE_REMOVE(s);
        yalx_free_stack(s);
        free(s);
    }
    yalx_mutex_final(&pool->mutex);
    memset(pool, 0, sizeof(*pool));
}
