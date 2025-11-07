#include "vm/frame.h"
#include "threads/malloc.h"

static struct list f_tables;
static struct lock f_lock;


void frame_init(void) 
{
    list_init (&f_tables);
    lock_init (&f_lock);
}

frame* get_frame(void *upage)  
{
    lock_acquire (&f_lock);
    void *kpage = palloc_get_page(PAL_USER);
    if (kpage == NULL) 
      { 
        //need to evict
        if (!evict_frame()) {
          lock_release(&f_lock);
          palloc_free_page(kpage);
          return NULL;
        }
      }
    frame *f = malloc (sizeof(frame));
    f->kpage = kpage;
    f->upage = upage;
}