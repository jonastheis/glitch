#include <stdint.h>

typedef struct {
    uint64_t pfn : 54;
    unsigned int soft_dirty : 1;
    unsigned int file_page : 1;
    unsigned int swapped : 1;
    unsigned int present : 1;
} PagemapEntry;

typedef struct {
    uint32_t useraddr;
    char usage[20];
    int id;
    int alloc_order;
    int texture_id;
    uint64_t pfn;
} KGSLEntry;

int pagemap_get_entry(PagemapEntry *entry, uintptr_t vaddr);
