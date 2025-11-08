#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <list.h>
#include <stdbool.h>
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/palloc.h"

void frame_init (void);
struct frame_info* get_frame (void *upage);
bool free_frame (void *kpage);
void evict_frame (void);

typedef struct frame_info 
{
    void *kpage;
    void *upage;
    struct thread *thread;
    struct list_elem elem;
    bool is_pinned;
} frame;

#endif