#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

void memory_init(void);
void memory_print_stats(void);
void memory_test(void);

void *kmalloc(size_t size);
void kfree(void *ptr);

#endif
