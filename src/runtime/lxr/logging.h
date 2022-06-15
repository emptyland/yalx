#pragma once
#ifndef YALX_RUNTIME_LXR_LOGGING_H_
#define YALX_RUNTIME_LXR_LOGGING_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TOP_STRIPES_SHIFT 14
#define TOP_STRIPES_SIZE (1u << TOP_STRIPES_SHIFT)
#define TOP_STRIPES_MASK ((1ull << TOP_STRIPES_SHIFT) - 1)

#define SECONDARY_STRIPES_SHIFT 11
#define SECONDARY_STRIPES_SIZE (1u << SECONDARY_STRIPES_SHIFT)
#define SECONDARY_STRIPES_MASK ((1ull << SECONDARY_STRIPES_SHIFT) - 1)

#define CARD_SHIFT 9
#define CARD_SIZE (1u << CARD_SHIFT)
#define CARD_MASK ((1ull << CARD_SHIFT) - 1)

struct yalx_value_any;

struct lxr_log_stripe {
    void *upper;
    void *lower;
    union {
        _Atomic uint8_t bytes[1];
        struct lxr_log_stripe *_Atomic next[1];
    } u;
}; // struct lxr_log_stripe

struct lxr_log_node {
    struct lxr_log_node *_Atomic next;
    void *data;
}; // struct lxr_log_node

struct lxr_log_queue {
    struct lxr_log_node *_Atomic head;
    struct {
        uint8_t *block;
        uint8_t *_Atomic free;
    } chunk[2];
}; // struct lxr_log_queue

struct lxr_fields_logger {
    struct lxr_log_stripe *_Atomic *top_stripes;
    _Atomic size_t used_memory_in_bytes;
    struct lxr_log_queue decrments;
    struct lxr_log_queue modification;
}; // struct lxr_fields_logger

int lxr_init_fields_logger(struct lxr_fields_logger *logger);
void lxr_free_fields_logger(struct lxr_fields_logger *logger);

int lxr_attempt_to_log(struct lxr_fields_logger *logger, void *address);
int lxr_attempt_to_unlog(struct lxr_fields_logger *logger, void *address);

int lxr_has_logged(struct lxr_fields_logger *logger, void *address);

int lxr_init_log_queue(struct lxr_log_queue *queue);
void lxr_free_log_queue(struct lxr_log_queue *queue);

struct lxr_log_node *lxr_log_queue_push(struct lxr_log_queue *queue, void *data);
struct lxr_log_node *lxr_log_queue_take(struct lxr_log_queue *queue);

void lxr_log_queue_clear(struct lxr_log_queue *queue);

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_LXR_LOGGING_H_
