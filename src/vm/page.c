#include "vm/page.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"

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
    sp *supp = malloc (sizeof (sp));
    if (!supp) 
      {
        return false;
      }
    supp->file = NULL;
    supp->offset_val = 0;
    supp->bytes = 0;
    supp->padding_bytes = PGSIZE;
    supp->is_writeable = writable;
    supp->is_loaded = false;
    supp->loc = SP_LOC_ZERO;
    supp->vm_page = pg_round_down (upage);
    supp->swap = SIZE_MAX;
    return hash_insert (&table->htable, &supp->elem) == NULL;
}
bool mark_swapped_spt (spt *table, void *upage, size_t slot)
{
    lock_acquire (&table->spt_lock);
    sp *supp;
    supp->vm_page = pg_round_down (upage);
    struct hash_elem *he = hash_find (&table->htable, &supp->elem);
    if (!he) 
      {
        lock_release (&table->spt_lock);
        return false;
      }
    sp *supp = hash_entry (he, sp, elem);
    supp->is_loaded = false;
    supp->swap = slot;
    supp->loc = SP_LOC_SWAP;
    lock_release (&table->spt_lock);
    return true;
}