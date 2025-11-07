#include "vm/page.h"
#include "threads/malloc.h"

/* Returns a hash value for page p. */
unsigned
page_hash (const struct hash_elem *p_, void *aux UNUSED)
{
  sp *p = hash_entry (p_, sp, elem);
  return hash_bytes (&p->vm_page, sizeof p->vm_page);
}

/* Returns true if page a precedes page b. */
bool
page_less (const struct hash_elem *a_, const struct hash_elem *b_,
           void *aux UNUSED)
{
  const sp *a = hash_entry (a_, sp, elem);
  const sp *b = hash_entry (b_, sp, elem);

  return a->vm_page < b->vm_page;
}

void init_spt (spt *table) {
    hash_init (&table->htable, page_hash, page_less, NULL);
}

void init_spt (spt *to_initialize)
{
    return;
}
void del_spt (spt *to_destroy)
{
    return;
}

sp *search_spt (spt *table, const void *upage)
{
    return NULL;
}
bool insert_file_spt (spt *table, void *upage, struct file *file,
                     off_t offset, size_t bytes, size_t zero_bytes, 
                     bool writable)
{
    return false;
}

bool insert_zero_spt (spt *table, void *upage, bool writable)
{
    return false;
}
bool mark_swapped_spt (spt *table, void *upage, size_t slot)
{
    return false;
}