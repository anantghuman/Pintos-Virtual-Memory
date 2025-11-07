#include <hash.h>
#include <stdbool.h>
#include "filesys/file.h"

typedef struct supplemental_page_table {
    void *vm_page; 
    struct file *file;
    off_t offset_val;
    size_t bytes;
    int location;
    bool is_writeable;
    bool is_loaded;
    struct hash_elem elem;
    size_t swap;
} spt;