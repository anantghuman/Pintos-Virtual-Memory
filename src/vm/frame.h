#include <list.h>
#include <stdbool.h>
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/palloc.h"

static struct list f_tables;
static struct lock f_lock;


void frame_init (void);
frame* get_frame ();
bool free_frame (void *kpage);
frame* evict_frame (void);

typedef struct frame_info 
{
    void *kpage;
    void *upage;
    struct thread *page;
    struct list_elem elem;
    bool is_pinned;
} frame;