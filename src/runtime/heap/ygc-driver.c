#include "runtime/heap/ygc-driver.h"
#include "runtime/heap/ygc.h"
#include "runtime/heap/heap.h"
#include "runtime/utils.h"


void ygc_gc_sync(struct heap *h, struct collected_statistics *stat) {
    stat->total_mills = yalx_current_mills_in_precision();
    stat->pause_mills = 0;

    struct ygc_core *ygc = ygc_heap_of(h);
    stat->collected_rss_in_bytes = ygc->rss;

    // Stage 1: Paused mark start
    ygc_mark_start(h);
    ygc_marking_tls_commit(&ygc->mark, yalx_os_thread_self());

    // Stage 2: Concurrent mark
    ygc_mark(h, 1);

    // Stage 3: Concurrent reset relocation set
    ygc_reset_relocation_set(h);

    // Stage 4: Concurrent select relocation set
    ygc_select_relocation_set(ygc);

    // Stage 5: Paused relocate start (relocate roots)
    ygc_relocate_start(h);

    // Stage 6: Concurrent relocate
    ygc_relocate(h);

    stat->total_mills = yalx_current_mills_in_precision() - stat->total_mills;
}