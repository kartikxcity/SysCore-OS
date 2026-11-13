#ifndef RESOURCE_H
#define RESOURCE_H

#include <stddef.h>

#define MAX_RESOURCES 32

void resource_init(void);

int resource_allocate(int task_id, size_t size);
int resource_release(int resource_id);

void resource_list(void);
void resource_release_all(int task_id);

#endif
