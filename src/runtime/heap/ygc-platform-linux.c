#include "runtime/heap/ygc.h"
#include "runtime/checking.h"
#define _GNU_SOURCE
#define __USE_GNU
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

const char *tmpfs_mount_points[] = {
        "/run/shm",
        "/dev/shm",
        NULL,
};

int create_file_fd(const char *name) {
    // Try to create an anonymous file using the O_TMPFILE flag. Note that this
    // flag requires kernel >= 3.11. If this fails we fall back to open/unlink.
    const int fd_anon = open(tmpfs_mount_points[0], O_TMPFILE|O_EXCL|O_RDWR|O_CLOEXEC, S_IRUSR|S_IWUSR);
    if (fd_anon < 0) {
        PLOG("Failed to create anonymous file in %s", tmpfs_mount_points[0]);
    } else {
        return fd_anon;
    }

    char full_name[260] = {0};
    snprintf(full_name, arraysize(full_name), "%s/%s.%d", tmpfs_mount_points[0], name, getpid());
    // Create file
    const int fd = open(full_name, O_CREAT|O_EXCL|O_RDWR|O_CLOEXEC, S_IRUSR|S_IWUSR);
    if (fd < 0) {
        PLOG("Failed to create file: %s", full_name);
        return -1;
    }

    if (unlink(full_name) < 0) {
        PLOG("Failed to unlink file: %s", full_name);
        return -1;
    }

    return fd;
}

int memory_backing_init(struct memory_backing *backing, size_t capacity) {
    memset(backing, 0, sizeof(*backing));

    backing->fd = memfd_create("ygc_page", MFD_ALLOW_SEALING);
    if (backing->fd < 0) {
        PLOG("Failed to memfd_create()");
        backing->fd = create_file_fd("ygc_page");
    }
    if (backing->fd < 0) {
        return -1;
    }

    if (ftruncate(backing->fd, (off_t)capacity) < 0) {
        PLOG("Failed to ftruncate %zd bytes", capacity);
        close(backing->fd);
        return -1;
    }

    backing->size = capacity;
    backing->refs = 1;
    return 0;
}

void memory_backing_final(struct memory_backing *backing) {
    close(backing->fd);
}

void memory_backing_map(struct memory_backing *backing, uintptr_t addr, size_t size, uintptr_t offset) {
    void *rs = mmap((void *)addr, size, PROT_WRITE|PROT_READ, MAP_FIXED|MAP_SHARED, backing->fd,
                    (off_t)offset);
    DCHECK(rs != MAP_FAILED);
}

void memory_backing_unmap(struct memory_backing *backing, uintptr_t addr, size_t size) {
    USE(backing);
    munmap((void *)addr, size);
}