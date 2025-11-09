#include "vm/swap.h"
#include "devices/block.h"
#include "lib/kernel/bitmap.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

struct swap_info *swap;

void swap_init ()
{
    swap = malloc (sizeof (swap));
    if (!swap)
      {
        thread_current ()->exit_stat = -1;
        thread_exit();
      }
    swap->swap_partition = block_get_role (BLOCK_SWAP);
    swap->bitmap = bitmap_create (block_size (swap->swap_partition) / 8);
    lock_init (&swap->s_lock);
}

void swap_write_sector (void *kpage, size_t *s)
{
    lock_acquire (&swap->s_lock);
    size_t slot = bitmap_scan_and_flip (swap->bitmap, 0, 1, false);
    if (slot == BITMAP_ERROR)
      {
        lock_release (&swap->s_lock);
        thread_current ()->exit_stat = -1;
        thread_exit();
      }
    lock_release (&swap->s_lock);
    size_t temp = 0;
    while (temp < (PGSIZE/BLOCK_SECTOR_SIZE))
      {
        block_write (swap->swap_partition, slot * (PGSIZE/BLOCK_SECTOR_SIZE) + temp, 
                     (uint8_t *) kpage + temp * BLOCK_SECTOR_SIZE);
        temp++;
      }
    return slot;
}

void swap_read_sector (void *kpage, size_t s)
{
    lock_acquire (&swap->s_lock);
    for (size_t i = 0; i < (PGSIZE/BLOCK_SECTOR_SIZE); i++)
      {
        block_read (swap->swap_partition, s * (PGSIZE/BLOCK_SECTOR_SIZE) + i, 
                    (uint8_t *) kpage + i * BLOCK_SECTOR_SIZE);
      }
    bitmap_reset (swap->bitmap, s);
    lock_release (&swap->s_lock);
}

void swap_free_sector (size_t s)
{
    lock_acquire (&swap->s_lock);
    if (swap == NULL || swap->swap_partition == NULL)
      {
        lock_release (&swap->s_lock);
        return;
      }
    bitmap_reset (swap->bitmap, s);
    lock_release (&swap->s_lock);
}



