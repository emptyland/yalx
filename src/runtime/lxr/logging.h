#pragma once
#ifndef YALX_RUNTIME_LXR_LOGGING_H_
#define YALX_RUNTIME_LXR_LOGGING_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TOP_STRIPES_SHIFT 12
#define TOP_STRIPES_SIZE (1u << TOP_STRIPES_SHIFT)
#define TOP_STRIPES_MASK ((1ull << TOP_STRIPES_SHIFT) - 1)

#define SECONDARY_STRIPES_SHIFT 11
#define SECONDARY_STRIPES_SIZE (1u << SECONDARY_STRIPES_SHIFT)
#define SECONDARY_STRIPES_MASK ((1ull << SECONDARY_STRIPES_SHIFT) - 1)

#define CARD_SHIFT 9
#define CARD_SIZE (1u << CARD_SHIFT)
#define CARD_MASK ((1ull << CARD_SHIFT) - 1)

struct lxr_log_stripe {
    void *upper;
    void *lower;
    union {
        _Atomic uint8_t bytes[1];
        struct lxr_log_stripe *_Atomic next[1];
    } u;
};

struct lxr_fields_logger {
    struct lxr_log_stripe *_Atomic top_stripes[TOP_STRIPES_SIZE];
    _Atomic size_t used_memory_in_bytes;
};

int lxr_init_fields_logger(struct lxr_fields_logger *logger);
void lxr_free_fields_logger(struct lxr_fields_logger *logger);

int lxr_attempt_to_log(struct lxr_fields_logger *logger, void *address);
int lxr_has_logged(struct lxr_fields_logger *logger, void *address);

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_LXR_LOGGING_H_
