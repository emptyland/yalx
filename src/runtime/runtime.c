#include "runtime/runtime.h"
#include "runtime/process.h"
#include <unistd.h>
#include <sys/sysctl.h>
#include <stdlib.h>

int ncpus = 0;

int os_page_size = 0;

const struct yalx_str yalx_version = YALX_STR("yalx-lang v0.0.0");

struct processor *procs = NULL;

int nprocs = 0;

struct machine m0;

_Thread_local struct machine *thread_local_mach;

int yalx_runtime_init() {
    
#if defined(YALX_OS_DARWIN)
    os_page_size = getpagesize();
    
    int mib[2] = { CTL_HW, HW_AVAILCPU };
    unsigned cpu_count;
    size_t size_of_cpu_count = sizeof(cpu_count);
    int status = sysctl(mib, 2, &cpu_count, &size_of_cpu_count, NULL, 0);
    if (status != 0) {
        return -1;
    }
    ncpus = (int)cpu_count;
#endif // defined(YALX_OS_DARWIN)
    
    m0.next = &m0;
    m0.prev = &m0;
    m0.thread = pthread_self();
    m0.state = MACH_INIT;
    
    thread_local_mach = &m0;

    nprocs = ncpus;
    procs = malloc(nprocs * sizeof(struct processor));
    if (!procs) {
        return -1;
    }
    
    for (int i = 0; i < nprocs; i++) {
        procid_t id = {i};
        if (yalx_init_processor(id, &procs[i]) < 0) {
            return -1;
        }
    }
    
    yalx_add_machine_to_processor(&procs[0], &m0);
    return 0;
}


void *fill_memory_zag(void *chunk, size_t n, uint32_t zag) {
    if (!chunk) {
        return NULL;
    }
    uint32_t *p = (uint32_t *)chunk;
    for (int i = 0; i < n / sizeof(zag); i++) {
        *p++ = zag;
    }
    const size_t r = n % sizeof(zag);
    address_t x = ((address_t)chunk) + n - r;
    switch (r) {
        case 1:
            *x = ((address_t)&zag)[0];
            break;
        case 2:
            *x++ = ((address_t)&zag)[0];
            *x++ = ((address_t)&zag)[1];
            break;
        case 3:
            *x++ = ((address_t)&zag)[0];
            *x++ = ((address_t)&zag)[1];
            *x++ = ((address_t)&zag)[2];
            break;
            
        case 0:
        default:
            break;
    }
    return chunk;
}
