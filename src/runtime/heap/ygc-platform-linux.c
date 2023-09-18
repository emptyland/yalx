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
        perror("Failed to create anonymous file in ");
    } else {
        return fd_anon;
    }

    char full_name[260] = {0};
    snprintf(full_name, arraysize(full_name), "%s/%s.%d", tmpfs_mount_points[0], name, getpid());
    // Create file
    const int fd = open(full_name, O_CREAT|O_EXCL|O_RDWR|O_CLOEXEC, S_IRUSR|S_IWUSR);
    if (fd < 0) {
        perror("Failed to create file");
        return -1;
    }

    if (unlink(full_name) < 0) {
        perror("Failed to unlink file");
        return -1;
    }

    return fd;
}

struct os_page *allocate_os_page(size_t size) {
    struct os_page *page = MALLOC(struct os_page);
    if (!page) {
        return NULL;
    }
    page->next = page;
    page->prev = page;
    page->addr = NULL;
    page->refs = 0;

    page->fd = memfd_create("page", MFD_ALLOW_SEALING);
    if (page->fd < 0) {
        perror("memfd_create() fail");
    }
    page->fd = create_file_fd("page");
    if (page->fd < 0) {
        goto fail;
    }
    if (ftruncate(page->fd, (off_t)size) < 0) {
        goto fail;
    }
    page->size = size;
    page->addr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE, page->fd, 0);
    if (page->addr == MAP_FAILED) {
        goto fail;
    }

    return page;
fail:
    if (page->addr != NULL && page->addr != MAP_FAILED) {
        munmap(page->addr, size);
    }
    if (page->fd > 0) {
        close(page->fd);
    }
    free(page);
    return NULL;
}

void free_os_page(struct os_page *page) {
    if (!page) {
        return;
    }
    munmap(page->addr, page->size);
    close(page->fd);
    free(page);
}

address_t map_virtual_addr(struct os_page *page, linear_address_t virtual_addr) {
    void *addr = mmap((void *)virtual_addr.addr,
                      virtual_addr.size,
                      PROT_READ|PROT_WRITE,
                      MAP_FIXED,
                      page->fd, 0);
    if (addr == MAP_FAILED) {
        return NULL;
    }
    return (address_t)addr;
}