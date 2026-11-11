#ifndef TASK_H
#define TASK_H

#include <stddef.h>

#define MAX_TASKS 10
#define TASK_MEMORY_SIZE 256

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_TERMINATED
} TaskState;

typedef struct {
    int id;
    char name[16];
    TaskState state;
    int burst_time;
    int priority;
    int completion_time;
    int waiting_time;
    int turnaround_time;

    /* Phase 3: Task memory ownership */
    void *memory;
    size_t memory_size;

} Task;

void task_init(void);

int task_create(const char *name, int burst_time);
int task_create_priority(const char *name, int burst_time, int priority);

void task_list(void);
void task_memory_list(void);
void task_cleanup_memory(void);

void task_run_fcfs(void);
void task_run_sjf(void);
void task_run_priority(void);
void task_run_rr(void);
void task_reset(void);

#endif
