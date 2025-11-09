#include "vm/swap.h"
#include "devices/block.h"
#include "lib/kernel/bitmap.h"
#include "threads/synch.h"

struct swap_info *swap;

void swap_init ()
{
    swap = malloc (sizeof (swap));
    swap->swap_partition = block_get_role (BLOCK_SWAP);
    swap->bitmap = bitmap_create (block_size (swap->swap_partition) / 8);
    lock_init (&swap->s_lock);
}

void swap_write_sector (void *kpage, size_t *s)
{
    lock_acquire (&swap->s_lock);

    lock_release (&swap->s_lock);
}

void swap_read_sector (void *kpage, size_t s)
{
    lock_acquire (&swap->s_lock);

    lock_release (&swap->s_lock);
}

void swap_free_sector (size_t s)
{
    lock_acquire (&swap->s_lock);

    lock_release (&swap->s_lock);
}



