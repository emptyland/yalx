#include "runtime/heap/ygc.h"
#include "runtime/checking.h"

int memory_backing_init(struct memory_backing *backing, size_t capacity) {
    memset(backing, 0, sizeof(*backing));


    backing->size = capacity;
    backing->refs = 1;
    return 0;
}

void memory_backing_final(struct memory_backing *backing) {
    
}

void memory_backing_map(struct memory_backing *backing, uintptr_t addr, size_t size, uintptr_t offset) {
    
}

void memory_backing_unmap(struct memory_backing *backing, uintptr_t addr, size_t size) {
    
}
