#include <stddef.h>



void swap_init (void);
void swap_in (size_t slot, void *kpage);
size_t swap_out (void *kpage);
void swap_free (size_t slot);
