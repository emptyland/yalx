#include "runtime/heap/ygc-relocate.h"
#include "runtime/heap/ygc-forwarding.h"
#include "runtime/heap/ygc.h"
#include "runtime/runtime.h"
#include "runtime/checking.h"

void ygc_relocate_init(struct ygc_relocate *relocate) {
    yalx_job_init(&relocate->job, "yalx-relocating", ncpus / 2 < 4 ? 4 : ncpus / 2);
}

void ygc_relocate_final(struct ygc_relocate *relocate) {
    yalx_job_final(&relocate->job);
}

uintptr_t ygc_relocate_forward_object(struct ygc_relocate *relocate, struct forwarding *fwd, uintptr_t from_addr) {
    const uintptr_t from_offset = ygc_offset(from_addr);
    const uintptr_t from_index = (from_offset - fwd->virtual_addr.addr) >> pointer_shift_in_bytes;
    size_t dummy = 0;
    const struct forwarding_entry entry = forwarding_find(fwd, from_index, &dummy);

    DCHECK(entry.populated && "Should be forwarded");
    DCHECK(entry.from_index == from_index && "Should be forwarded");

    return ygc_good_address(entry.to_offset);
}