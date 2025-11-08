#include "vm/page.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"

/* Copied from pintos documentation */
unsigned
page_hash (const struct hash_elem *p_, void *aux UNUSED)
{
  sp *p = hash_entry (p_, sp, elem);
  return hash_bytes (&p->vm_page, sizeof p->vm_page);
}

/* Copied from pintos documentation */
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
    lock_init (&table->spt_lock);
}

void free_entry(struct hash_elem *e, void *aux UNUSED)
{
    free (hash_entry (e, sp, elem));
}

void del_spt (spt *to_destroy)
{
    hash_destroy (to_destroy, free_entry);
}

sp *search_spt (spt *table, const void *upage)
{
    // assuming the upage is the only key data
    sp *temp;
    temp->vm_page = upage;
    return hash_find(&table->htable, temp);
}
bool insert_file_spt (spt *table, void *upage, struct file *file,
                     off_t offset, size_t bytes, size_t padding, 
                     bool is_writable)
{
    sp *temp = malloc(sizeof(sp));
    if (temp == NULL)
      {
        return false;
      }
    temp->vm_page = pg_round_down(upage);
    temp->file = file;
    temp->offset_val = offset;
    temp->bytes = bytes;
    temp->padding_bytes = padding;
    temp->is_writeable = is_writable;
    temp->loc = 0;
    lock_acquire(&table->spt_lock);
    bool ret = hash_insert (&table->htable, &temp->elem) == NULL;
    lock_release(&table->spt_lock);
    return ret;
}

bool insert_zero_spt (spt *table, void *upage, bool is_writable)
{
    sp *temp = malloc (sizeof (sp));
    if (temp == NULL) 
      {
        return false;
      }
    temp->file = NULL;
    temp->offset_val = 0;
    temp->bytes = 0;
    temp->padding_bytes = PGSIZE;
    temp->is_writeable = is_writable;
    temp->is_loaded = false;
    temp->loc = 0;
    temp->vm_page = pg_round_down (upage);
    temp->swap = SIZE_MAX;
    lock_acquire(&table->spt_lock);
    bool ret = hash_insert (&table->htable, &temp->elem) == NULL;
    lock_release(&table->spt_lock);
    return ret;
}
bool mark_swapped_spt (spt *table, void *upage, size_t slot)
{
    lock_acquire (&table->spt_lock);
    sp *temp;
    temp->vm_page = pg_round_down (upage);
    struct hash_elem *t = hash_find (&table->htable, &temp->elem);
    if (!t) 
      {
        lock_release (&table->spt_lock);
        return false;
      }
    sp *supp = hash_entry (t, sp, elem);
    supp->is_loaded = false;
    supp->swap = slot;
    supp->loc = 1;
    lock_release (&table->spt_lock);
    return true;
}