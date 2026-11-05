#include <stdint.h>
#include <stddef.h>
#include "memory.h"

#define HEAP_SIZE (64 * 1024)
#define ALIGNMENT 16
#define BLOCK_MAGIC 0x53434F52

extern void print(const char *s);

typedef struct MemoryBlock {
    size_t size;
    int free;
    uint32_t magic;
    struct MemoryBlock *next;
} MemoryBlock;

/* A fixed 64 KB heap reserved inside the kernel. */
static uint8_t heap[HEAP_SIZE] __attribute__((aligned(16)));

static MemoryBlock *first_block = NULL;

static size_t align_size(size_t size)
{
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

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

void memory_init(void)
{
    first_block = (MemoryBlock *)heap;

    first_block->size =
        HEAP_SIZE - sizeof(MemoryBlock);

    first_block->free = 1;
    first_block->magic = BLOCK_MAGIC;
    first_block->next = NULL;
}

void *kmalloc(size_t size)
{
    if (size == 0 || first_block == NULL)
        return NULL;

    size = align_size(size);

    MemoryBlock *current = first_block;

    while (current != NULL) {

        if (current->magic != BLOCK_MAGIC)
            return NULL;

        if (current->free && current->size >= size) {

            /* Split the block if enough space remains. */
            if (current->size >=
                size + sizeof(MemoryBlock) + ALIGNMENT) {

                MemoryBlock *new_block =
                    (MemoryBlock *)((uint8_t *)(current + 1) + size);

                new_block->size =
                    current->size - size - sizeof(MemoryBlock);

                new_block->free = 1;
                new_block->magic = BLOCK_MAGIC;
                new_block->next = current->next;

                current->next = new_block;
                current->size = size;
            }

            current->free = 0;

            return (void *)(current + 1);
        }

        current = current->next;
    }

    return NULL;
}

void kfree(void *ptr)
{
    if (ptr == NULL || first_block == NULL)
        return;

    MemoryBlock *current = first_block;
    MemoryBlock *previous = NULL;

    while (current != NULL) {

        if (current->magic != BLOCK_MAGIC)
            return;

        if ((void *)(current + 1) == ptr) {

            if (current->free)
                return;

            current->free = 1;

            /* Merge with the next free block. */
            while (current->next != NULL &&
                   current->next->free) {

                MemoryBlock *next = current->next;

                current->size +=
                    sizeof(MemoryBlock) + next->size;

                current->next = next->next;
            }

            /* Merge with the previous free block. */
            if (previous != NULL && previous->free) {

                previous->size +=
                    sizeof(MemoryBlock) + current->size;

                previous->next = current->next;

                current = previous;

                /* Merge forward again after backward merging. */
                while (current->next != NULL &&
                       current->next->free) {

                    MemoryBlock *next = current->next;

                    current->size +=
                        sizeof(MemoryBlock) + next->size;

                    current->next = next->next;
                }
            }

            return;
        }

        previous = current;
        current = current->next;
    }
}
void memory_print_stats(void)
{
    size_t used = 0;
    size_t free_space = 0;

    MemoryBlock *current = first_block;

    while (current != NULL) {

        if (current->magic != BLOCK_MAGIC) {
            print("\nMemory metadata error!");
            return;
        }

        if (current->free)
            free_space += current->size;
        else
            used += current->size;

        current = current->next;
    }

    print("\nKernel Heap Size: ");
    print_number(HEAP_SIZE);
    print(" bytes");

    print("\nAllocated: ");
    print_number(used);
    print(" bytes");

    print("\nFree: ");
    print_number(free_space);
    print(" bytes");

    print("\n");
}

void memory_test(void)
{
    print("\nStarting memory allocation test...");

    void *a = kmalloc(128);
    void *b = kmalloc(256);

    if (a == NULL || b == NULL) {
        print("\nInitial allocation failed.");
        kfree(a);
        kfree(b);
        return;
    }

    print("\nTwo memory blocks allocated successfully.");

    kfree(a);

    print("\nFirst block released.");

    void *c = kmalloc(64);

    if (c == NULL) {
        print("\nReuse allocation failed.");
        kfree(b);
        return;
    }

    print("\nFreed memory reused successfully.");

    kfree(c);
    kfree(b);

    print("\nAll test allocations released.");

    memory_print_stats();
}
