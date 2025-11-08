#include "vm/page.h"
#include "vm/frame.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"
#include "threads/synch.h"
#include "filesys/file.h"

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

bool move_page_to_frame (sp *entry) 
{
  void *upage = entry->vm_page;
  frame *f = get_frame ();
  if (f == NULL)
    {
      return false;
    }

  void *kpage = f->kpage; 

  bool check = false;
  
  // file system
  if (entry->loc == 0) 
    {
      file_seek (entry->file, entry->offset_val);
      off_t offset = file_read (entry->file, kpage, entry->bytes);
      if (offset != (off_t) entry->bytes) 
        {
          check = false;
        }

      memset ((uint8_t*) kpage + entry->bytes, 0, entry->padding_bytes);
      check = true;
    }
  // swap
  if (entry->loc == 1) 
    {
      return false;
    }
  // zero page
  if (entry->loc == 2)
    {
      memset (kpage, 0, PGSIZE);
      check = true;
    }

  if (check)
    {
      struct thread *curr = f->thread;
      if (!pagedir_set_page (curr->pagedir, entry->vm_page, f, entry->is_writeable))
      {
        free_frame (f);
        return false;
      }
      entry->is_loaded = true;
      entry->loc = 3;
    }
    else 
    {
      free_frame (f);
      return false;
    }
    return true;
}

void del_spt (spt *to_destroy)
{
    hash_destroy (&to_destroy->htable, free_entry);
}

sp *search_spt (spt *table, const void *upage)
{
    // assuming the upage is the only key data
    struct hash_elem *he;
    sp temp;
    temp.vm_page = pg_round_down (upage);
    lock_acquire(&table->spt_lock);
    he = hash_find (&table->htable, &temp.elem);
    lock_release(&table->spt_lock);
    return he ? hash_entry (he, sp, elem) : NULL;
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
    sp temp;
    temp.vm_page = pg_round_down (upage);
    struct hash_elem *t = hash_find (&table->htable, &temp.elem);
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