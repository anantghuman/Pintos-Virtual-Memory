#ifndef VM_PAGE_H
#define VM_PAGE_H


#include <hash.h>
#include <stdbool.h>
#include "filesys/file.h"
#include "threads/synch.h"

enum sp_loc 
{
    SP_LOC_FILE = 0,
    SP_LOC_SWAP = 1,
    SP_LOC_ZERO = 2
};

typedef struct supplemental_page 
{
    void *vm_page; 
    struct file *file;
    off_t offset_val;
    size_t bytes;
    int location;
    bool is_writeable;
    bool is_loaded;
    struct hash_elem elem;
    size_t swap;
} sp;

typedef struct supplemental_page_table
{
    struct hash htable;
    struct lock spt_lock;   
} spt;

void init_spt (spt *to_initialize);
void del_spt (spt *to_destroy);

sp *search_spt (spt *table, const void *upage);
bool insert_file_spt (spt *table, void *upage, struct file *file,
                     off_t offset, size_t bytes, size_t zero_bytes, 
                     bool writable);

bool insert_zero_spt (spt *table, void *upage, bool writable);
bool mark_swapped_spt (spt *table, void *upage, size_t slot);

#endif