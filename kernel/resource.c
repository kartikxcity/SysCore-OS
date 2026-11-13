#include <stddef.h>
#include "resource.h"
#include "memory.h"
#include "task.h"

extern void print(const char *s);

typedef struct {
    int resource_id;
    int task_id;
    void *address;
    size_t size;
    int active;
} Resource;

static Resource resources[MAX_RESOURCES];
static int next_resource_id = 1;

static void print_number(size_t n)
{
    char buffer[12];
    int i = 0;

    if (n == 0) {
        print("0");
        return;
    }

    while (n > 0 && i < 11) {
        buffer[i++] = '0' + (n % 10);
        n /= 10;
    }

    while (i > 0) {
        char c[2] = { buffer[--i], '\0' };
        print(c);
    }
}

void resource_init(void)
{
    for (int i = 0; i < MAX_RESOURCES; i++) {
        resources[i].resource_id = 0;
        resources[i].task_id = 0;
        resources[i].address = NULL;
        resources[i].size = 0;
        resources[i].active = 0;
    }

    next_resource_id = 1;
}

int resource_allocate(int task_id, size_t size)
{
    if (!task_exists(task_id) || size == 0)
        return -1;

    int slot = -1;

    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (!resources[i].active) {
            slot = i;
            break;
        }
    }

    if (slot == -1)
        return -1;

    void *memory = kmalloc(size);

    if (memory == NULL)
        return -1;

    Resource *r = &resources[slot];

    r->resource_id = next_resource_id++;
    r->task_id = task_id;
    r->address = memory;
    r->size = size;
    r->active = 1;

    return r->resource_id;
}

int resource_release(int resource_id)
{
    for (int i = 0; i < MAX_RESOURCES; i++) {
        Resource *r = &resources[i];

        if (r->active && r->resource_id == resource_id) {
            kfree(r->address);

            r->address = NULL;
            r->size = 0;
            r->task_id = 0;
            r->active = 0;

            return 1;
        }
    }

    return 0;
}

void resource_release_all(int task_id)
{
    for (int i = 0; i < MAX_RESOURCES; i++) {
        Resource *r = &resources[i];

        if (r->active && r->task_id == task_id) {
            kfree(r->address);

            r->address = NULL;
            r->size = 0;
            r->task_id = 0;
            r->active = 0;
        }
    }
}

void resource_list(void)
{
    int found = 0;
    size_t total = 0;

    print("\nRESOURCE TABLE");
    print("\nID   TASK   SIZE");

    for (int i = 0; i < MAX_RESOURCES; i++) {
        Resource *r = &resources[i];

        if (!r->active)
            continue;

        found = 1;
        total += r->size;

        print("\n");
        print_number((size_t)r->resource_id);
        print("    ");
        print_number((size_t)r->task_id);
        print("      ");
        print_number(r->size);
        print(" bytes");
    }

    if (!found)
        print("\nNo active resources.");

    print("\nTotal allocated: ");
    print_number(total);
    print(" bytes\n");
}
