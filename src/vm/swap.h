#ifndef VM_SWAP_H
#define VM_SWAP_H


#include "vm/page.h"
#include "vm/frame.h"


extern struct lock file_lock;

typedef struct swap_info {
    struct block *swap_partition;
    struct bitmap *bitmap;
    struct lock s_lock;
} sw;

void swap_init ();
size_t swap_write_sectors (void *kpage);
void swap_read_sectors (void *kpage, size_t s);
void swap_free_sectors (size_t s);

#endif