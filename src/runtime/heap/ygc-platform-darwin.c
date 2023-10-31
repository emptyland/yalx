#include "runtime/heap/ygc.h"
#include "runtime/checking.h"
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <sys/mman.h>
#include <sys/types.h>

static int mremap(uintptr_t from_addr, uintptr_t to_addr, size_t size) {
  mach_vm_address_t remap_addr = to_addr;
  vm_prot_t remap_cur_prot;
  vm_prot_t remap_max_prot;

  // Remap memory to an additional location
  const kern_return_t res = mach_vm_remap(mach_task_self(),
                                          &remap_addr,
                                          size,
                                          0 /* mask */,
                                          VM_FLAGS_FIXED | VM_FLAGS_OVERWRITE,
                                          mach_task_self(),
                                          from_addr,
                                          FALSE /* copy */,
                                          &remap_cur_prot,
                                          &remap_max_prot,
                                          VM_INHERIT_COPY);

  return (res == KERN_SUCCESS) ? 0 : -1;
}

int memory_backing_init(struct memory_backing *backing, size_t capacity) {
    memset(backing, 0, sizeof(*backing));

    void *const rs = mmap(NULL, capacity, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_NORESERVE | MAP_ANONYMOUS, -1, 0);
    if (rs == MAP_FAILED) {
        PLOG("backing mmap() fail!");
        return -1;
    }
    
    backing->base = (uintptr_t)rs;
    backing->size = capacity;
    backing->refs = 1;
    return 0;
}

void memory_backing_final(struct memory_backing *backing) {
    munmap((void *)backing->base, backing->size);
}

void memory_backing_map(struct memory_backing *backing, uintptr_t addr, size_t size, uintptr_t offset) {
//    const void* const ptr = mmap((void*)backing->base + offset, size, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
//    DCHECK(ptr != MAP_FAILED);
    int rs = mremap(backing->base + offset, addr, size);
    DCHECK(rs == 0);

}

void memory_backing_unmap(struct memory_backing *backing, uintptr_t addr, size_t size) {
    // Note that we must keep the address space reservation intact and just detach
    // the backing memory. For this reason we map a new anonymous, non-accessible
    // and non-reserved page over the mapping instead of actually unmapping.
    const void* const rs = mmap((void*)addr, size, PROT_NONE, MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE, -1, 0);
    if (rs == MAP_FAILED) {
        DLOG(FATAL, "Backing unmap fail!");
    }
}
