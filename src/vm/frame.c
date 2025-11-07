#include "vm/frame.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"

struct list f_table;
struct lock f_lock;
struct list_elem *clock_hand;

void frame_init(void) 
{
    list_init (&f_tables);
    lock_init (&f_lock);
    clock_hand = NULL;
}

frame* get_frame(void *upage)  
{
    lock_acquire (&f_lock);
    void *kpage = palloc_get_page(PAL_USER);
    if (kpage == NULL) 
      { 
        if (!evict_frame()) {
          lock_release(&f_lock);
          palloc_free_page(kpage);
          return NULL;
        }
      }
    frame *f = malloc (sizeof(frame));
    f->kpage = kpage;
    f->upage = upage;
    // f->is_pinned = true;
    list_push_back(&f_table, f);
    return f;
}

bool evict_frame() 
{
    if (clock_hand == NULL) 
      {
        clock_hand = list_begin(&f_table);
      }
    bool evicted = false;
    while (!evicted) 
      {
        frame *current = list_entry(clock_hand, frame, elem);
        if (pagedir_is_accessed (current->thread->pagedir, current->upage))
          {
            pagedir_set_accessed (current->thread->pagedir, current->upage, false);
          }
        else 
          {
            free_frame(current);
            evicted = true;
          }
        clock_hand = list_next(clock_hand);
        if (clock_hand == list_end(&f_table))
          {
            clock_hand = list_begin(&f_table);
          }
      }
}