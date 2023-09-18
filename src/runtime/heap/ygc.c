#include "runtime/heap/ygc.h"

int ygc_init(struct ygc_core *ygc) {
    ygc->os_pages.prev = &ygc->os_pages;
    ygc->os_pages.next = &ygc->os_pages;
    ygc->rss = 0;
    return 0;
}

void ygc_final(struct ygc_core *ygc) {
    while (!QUEUE_EMPTY(&ygc->os_pages)) {
        struct os_page *page = ygc->os_pages.next;
        QUEUE_REMOVE(page);
        free_os_page(page);
    }
    ygc->rss = 0;
}