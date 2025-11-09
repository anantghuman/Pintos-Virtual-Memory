#include "vm/frame.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "vm/swap.h"

struct list f_table;
struct lock f_lock;
struct list_elem *clock_hand;

void frame_init(void) 
{
    list_init (&f_table);
    lock_init (&f_lock);
    clock_hand = NULL;
}

frame *find_frame (void *upage) 
{
  lock_acquire (&f_lock);
  struct list_elem *temp = list_begin (&f_table);
  while (temp != list_end (&f_table)) 
    {
      frame *f = list_entry (temp, frame, elem);
      if (f->upage == upage && f->thread == thread_current())
        {
          lock_release (&f_lock);
          return NULL;
        }
      temp = list_next (temp);
    }
  lock_release (&f_lock);
  return NULL;
}

void pin_buffer_frames (void *buf_ptr, unsigned *size_ptr) 
{
  uint8_t *page = pg_round_down (buf_ptr);
  frame *f;
  while (page < (uint8_t *) buf_ptr + *size_ptr) 
    {
      sp *temp = search_spt (&thread_current ()->spt, (void*) page);
      if (pagedir_get_page (thread_current ()->pagedir, page) == NULL && temp)
        {
          if (!temp || !move_page_to_frame (temp)) 
            {
              return;
            }
        }
      f = find_frame (page);
      if (f != NULL)
        {
          f->is_pinned = true;
        }
      page += PGSIZE;
    }
}

void unpin_buffer_frames (void *buf_ptr , unsigned *size_ptr)
  {
    uint8_t *page = pg_round_down (buf_ptr);
    frame *f;
    while (page < (uint8_t *) buf_ptr + *size_ptr) 
      {
        f = find_frame (page);
        if (f != NULL) 
          {
            f->is_pinned = false;
          }
        page += PGSIZE;
      }
  }

struct frame_info* get_frame(void *upage)  
{
    lock_acquire (&f_lock);
    void *kpage = palloc_get_page(PAL_USER);
    if (kpage == NULL) 
      {
        evict_frame();
        kpage = palloc_get_page(PAL_USER);
        if (kpage == NULL) {
          lock_release(&f_lock);
          return NULL;
        }
      }
    frame *f = malloc (sizeof(frame));
    f->kpage = kpage;
    f->upage = upage;
    f->thread = thread_current();
    f->is_pinned = false;
    list_push_back (&f_table, &f->elem);
    if (!clock_hand) 
      {
        clock_hand = list_begin(&f_table);
      }
    lock_release (&f_lock);
    return f;
}

void evict_frame() 
{
    if (list_empty (&f_table))
      {
        return;
      }
    if (clock_hand == NULL) 
      {
        clock_hand = list_begin (&f_table);
      }
    bool evicted = false;
    while (!evicted) 
      {
        frame *current = list_entry(clock_hand, frame, elem);
        if (pagedir_is_accessed (current->thread->pagedir, current->upage))
          {
            pagedir_set_accessed (current->thread->pagedir, current->upage, false);
          }
        else if (!current->is_pinned)
          {
            sp *supp = search_spt (&current->thread->spt, current->upage);
            bool dirty = pagedir_is_dirty (current->thread->pagedir, current->upage);
            if (!(supp != NULL && (supp->loc == 0 && !dirty))) 
              {
                size_t slot = swap_write_sectors (current->kpage);
                mark_swapped_spt (&current->thread->spt, current->upage, slot);
              } 
            else 
              {
                lock_acquire(&current->thread->spt.spt_lock);
                supp->is_in_frame = false;
                lock_release(&current->thread->spt.spt_lock);
              }
            clock_hand = list_next(clock_hand);
            if (clock_hand == list_end(&f_table))
              {
                clock_hand = list_begin(&f_table);
              }
            pagedir_clear_page (current->thread->pagedir, current->upage);
            palloc_free_page (current->kpage);  
            list_remove (&current->elem);
            free(current);
            evicted = true;
          }
        clock_hand = list_next(clock_hand);
        if (clock_hand == list_end(&f_table))
          {
            clock_hand = list_begin(&f_table);
          }
      }
}

bool free_frame (void *kpage) 
{
    lock_acquire(&f_lock);
    bool check = false;
    struct list_elem *temp = list_begin(&f_table);
    while (temp != list_end(&f_table)) 
      {
        frame* f = list_entry(temp, frame, elem);
        if (f->kpage == kpage) 
          {
            check = true;
            if (clock_hand == &f->elem) 
              {
                clock_hand = list_next (clock_hand);
                if (clock_hand == list_end (&f_table)) 
                  {
                    clock_hand = list_begin (&f_table);
                  }
              }
            list_remove (&f->elem);
            palloc_free_page (f->kpage);
            free(f);
            break;
          }
        temp = list_next (temp);
      }
    lock_release(&f_lock);
    return check;
}