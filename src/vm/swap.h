#include "vm/page.h"
#include "vm/frame.h"


extern struct lock file_lock;

typedef struct swap_info {
    struct block *swap_partition;
    struct bitmap *bitmap;
    struct lock s_lock;
} swap;

void swap_init ();
void swap_write_sector (void *kpage, size_t *s);
void swap_read_sector (void *kpage, size_t s);
void swap_free_sector (size_t s);
