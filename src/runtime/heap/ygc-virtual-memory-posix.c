#include "runtime/heap/ygc.h"
#include "runtime/checking.h"
#include "runtime/runtime.h"
#include <sys/mman.h>

void release_mapping(uintptr_t addr, size_t size) {
    int rs = munmap((void *)addr, size);
    DCHECK(rs == 0);
}

static int probe_mapping(uintptr_t addr, size_t size) {
    void *rs = mmap((void*)addr, size, PROT_NONE, MAP_ANONYMOUS|MAP_PRIVATE|MAP_NORESERVE, -1, 0);
    if (rs == MAP_FAILED) {
        return 0; // Address space unusable
    }
    if (rs != (void *)addr) { // Failed to map
        release_mapping((uintptr_t)rs, size);
        return 0;
    }

    //DLOG(INFO, "[%lx,%lx) probe ok", addr, addr + size);
    return 1; // Ok
}

int reserve_continuous_memory(uintptr_t addr, size_t size) {
    uintptr_t marked0 = ygc_marked0(addr);
    uintptr_t marked1 = ygc_marked1(addr);
    uintptr_t remapped = ygc_remapped(addr);

    if (!probe_mapping(marked0, size)) {
        return 0;
    }
    if (!probe_mapping(marked1, size)) {
        release_mapping(marked0, size);
        return 0;
    }
    if (!probe_mapping(remapped, size)) {
        release_mapping(marked0, size);
        release_mapping(marked1, size);
        return 0;
    }
    return 1;
}