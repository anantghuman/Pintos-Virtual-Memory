#include "vm/frame.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"

struct list f_table;
struct lock f_lock;
struct list_elem *clock_hand;

void frame_init(void) 
{
    list_init (&f_table);
    lock_init (&f_lock);
    clock_hand = NULL;
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
            sp *supp = current->thread->spt;
            bool dirty = pagedir_is_dirty (current->thread->pagedir, current->upage);
            if (!(supp->loc == 0 && !dirty)) 
              {
                size_t slot = swap_out (current->kpage);
                supp->loc = 1;
                supp->swap = slot;
              } 
            list_remove (&current->elem);
            palloc_free_page (current->kpage);
            pagedir_clear_page (current->thread->pagedir, current->upage);
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