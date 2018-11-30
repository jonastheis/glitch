#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>

#include "mem_util.h"

int pagemap_get_entry(PagemapEntry *entry, uintptr_t vaddr) {
    int pagemap_fd = open("/proc/self/pagemap", O_RDONLY);
    size_t nread;
    size_t ret;
    uint64_t data;

    nread = 0;
    while (nread < sizeof(data)) {
        ret = pread(
            pagemap_fd,
            &data,
            sizeof(data),
            (vaddr / sysconf(_SC_PAGE_SIZE)) * sizeof(data) + nread
        );
        nread += ret;
        if (ret <= 0) {
            return 1;
        }
    }

    entry->pfn = data & (((uint64_t)1 << 54) - 1);
    entry->soft_dirty = (data >> 54) & 1;
    entry->file_page = (data >> 61) & 1;
    entry->swapped = (data >> 62) & 1;
    entry->present = (data >> 63) & 1;

    close(pagemap_fd);
    return 0;
}