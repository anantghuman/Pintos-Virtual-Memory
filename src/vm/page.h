#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include <stdbool.h>
#include "filesys/file.h"
#include "threads/synch.h"

typedef struct supplemental_page 
{
    void *vm_page; 
    struct file *file;
    off_t offset_val;
    size_t bytes;
    // 0 for file system, 1 for swap, 2 for zero, 3 for frame
    int loc;
    bool is_writeable;
    bool is_loaded;
    int padding_bytes;
    struct hash_elem elem;
    size_t swap;
} sp;

typedef struct supplemental_page_table
{
    struct hash htable;
    struct lock spt_lock;   
} spt;


unsigned page_hash (const struct hash_elem *p_, void *aux);
bool page_less (const struct hash_elem *a_, const struct hash_elem *b_, void *aux);
void init_spt (spt *to_initialize);
void free_entry(struct hash_elem * e, void *aux);
void del_spt (spt *to_destroy);

sp *search_spt (spt *table, const void *upage);
bool insert_file_spt (spt *table, void *upage, struct file *file,
                     off_t offset, size_t bytes, size_t zero_bytes, 
                     bool writable);

bool insert_zero_spt (spt *table, void *upage, bool writable);
bool mark_swapped_spt (spt *table, void *upage, size_t slot);

#endif