
#include "task.h"
#include "memory.h"

extern void print(const char *s);

static Task tasks[MAX_TASKS];
static int task_count = 0;
static int next_id = 1;

static void copy_name(char *dest, const char *src)
{
    int i = 0;

    if (!src)
        src = "Task";

    while (src[i] && i < 15) {
        dest[i] = src[i];
        i++;
    }

    dest[i] = '\0';
}

static void print_number(int n)
{
    char buf[12];
    int i = 0;

    if (n < 0) {
        print("-");
        n = -n;
    }

    if (n == 0) {
        print("0");
        return;
    }

    while (n > 0 && i < 11) {
        buf[i++] = '0' + n % 10;
        n /= 10;
    }

    while (i > 0) {
        char c[2] = { buf[--i], '\0' };
        print(c);
    }
}

static void print_state(TaskState state)
{
    if (state == TASK_READY)
        print("READY");
    else if (state == TASK_RUNNING)
        print("RUNNING");
    else
        print("TERMINATED");
}
int task_exists(int task_id)
{
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].id == task_id)
            return 1;
    }

    return 0;
}
void task_init(void)
{
    task_count = 0;
    next_id = 1;
}

int task_create_priority(const char *name, int burst_time, int priority)
{
    if (task_count >= MAX_TASKS ||
        burst_time <= 0 || priority <= 0)
        return -1;

    /* Allocate memory before creating the task. */
    void *task_memory = kmalloc(TASK_MEMORY_SIZE);

    if (task_memory == NULL)
        return -1;

    Task *t = &tasks[task_count];

    t->id = next_id++;
    copy_name(t->name, name);
    t->state = TASK_READY;
    t->burst_time = burst_time;
    t->priority = priority;

    t->completion_time = 0;
    t->waiting_time = 0;
    t->turnaround_time = 0;

    t->memory = task_memory;
    t->memory_size = TASK_MEMORY_SIZE;

    task_count++;

    return t->id;
}
int task_create(const char *name, int burst_time)
{
    return task_create_priority(name, burst_time, 1);
}

void task_list(void)
{
    if (task_count == 0) {
        print("\nNo tasks created. Use create first.");
        return;
    }

    print("\nID NAME            STATE       BURST PRIORITY");

    for (int i = 0; i < task_count; i++) {
        Task *t = &tasks[i];

        print("\n");
        print_number(t->id);
        print("  ");
        print(t->name);
        print("  ");
        print_state(t->state);
        print("  ");
        print_number(t->burst_time);
        print("     ");
        print_number(t->priority);
    }

    print("\n");
}
void task_memory_list(void)
{
    if (task_count == 0) {
        print("\nNo tasks available.");
        return;
    }

    print("\nTASK MEMORY USAGE");
    print("\nID   NAME           MEMORY");

    for (int i = 0; i < task_count; i++) {
        Task *t = &tasks[i];

        print("\n");
        print_number(t->id);
        print("    ");
        print(t->name);
        print("           ");

        print_number((int)t->memory_size);
        print(" bytes");
    }

    print("\n");
}

void task_cleanup_memory(void)
{
    int released = 0;

    for (int i = 0; i < task_count; i++) {
        Task *t = &tasks[i];

        if (t->memory != NULL) {
            kfree(t->memory);

            t->memory = NULL;
            t->memory_size = 0;

            released++;
        }
    }

    print("\nTask memory cleanup completed.");
    print("\nAllocations released: ");
    print_number(released);
    print("\n");
}
static void run_order(const char *title, int order[])
{
    int elapsed = 0;
    int total_wait = 0;
    int total_tat = 0;

    if (task_count == 0) {
        print("\nNo tasks available.");
        return;
    }

    print("\n");
    print(title);
    print("\nID BURST PRIORITY WAIT TAT CT");

    for (int i = 0; i < task_count; i++) {
        Task *t = &tasks[order[i]];

        t->state = TASK_RUNNING;
        t->waiting_time = elapsed;
        elapsed += t->burst_time;
        t->completion_time = elapsed;
        t->turnaround_time = elapsed;
        t->state = TASK_TERMINATED;

        total_wait += t->waiting_time;
        total_tat += t->turnaround_time;

        print("\n");
        print_number(t->id);
        print("   ");
        print_number(t->burst_time);
        print("     ");
        print_number(t->priority);
        print("      ");
        print_number(t->waiting_time);
        print("   ");
        print_number(t->turnaround_time);
        print("   ");
        print_number(t->completion_time);
    }

    print("\nAverage waiting time: ");
    print_number(total_wait * 100 / task_count);
    print(".");
    print_number((total_wait * 100 / task_count) % 100);

    print("\nAverage turnaround time: ");
    print_number(total_tat * 100 / task_count);
    print(".");
    print_number((total_tat * 100 / task_count) % 100);

    print("\n");
}

static void build_order(int order[])
{
    for (int i = 0; i < task_count; i++)
        order[i] = i;
}

void task_run_fcfs(void)
{
    int order[MAX_TASKS];
    build_order(order);
    run_order("FCFS Scheduling", order);
}

void task_run_sjf(void)
{
    int order[MAX_TASKS];
    build_order(order);

    for (int i = 0; i < task_count; i++) {
        for (int j = i + 1; j < task_count; j++) {
            if (tasks[order[j]].burst_time <
                tasks[order[i]].burst_time) {
                int temp = order[i];
                order[i] = order[j];
                order[j] = temp;
            }
        }
    }

    run_order("SJF Scheduling (Non-preemptive)", order);
}

void task_run_priority(void)
{
    int order[MAX_TASKS];
    build_order(order);

    for (int i = 0; i < task_count; i++) {
        for (int j = i + 1; j < task_count; j++) {
            if (tasks[order[j]].priority <
                tasks[order[i]].priority) {
                int temp = order[i];
                order[i] = order[j];
                order[j] = temp;
            }
        }
    }

    run_order("Priority Scheduling (Lower number = higher priority)",
              order);
}

void task_run_rr(void)
{
    int remaining[MAX_TASKS];
    int elapsed = 0;
    int completed = 0;
    int quantum = 2;
    int total_wait = 0;
    int total_tat = 0;

    if (task_count == 0) {
        print("\nNo tasks available.");
        return;
    }

    for (int i = 0; i < task_count; i++) {
        remaining[i] = tasks[i].burst_time;
        tasks[i].completion_time = 0;
        tasks[i].waiting_time = 0;
        tasks[i].turnaround_time = 0;
        tasks[i].state = TASK_READY;
    }

    print("\nRound Robin Scheduling (Quantum = 2)");
    print("\nExecution order: ");

    while (completed < task_count) {
        for (int i = 0; i < task_count; i++) {
            if (remaining[i] <= 0)
                continue;

            tasks[i].state = TASK_RUNNING;
            print("\nTask ");
            print_number(tasks[i].id);

            int slice = remaining[i] < quantum
                      ? remaining[i] : quantum;

            elapsed += slice;
            remaining[i] -= slice;

            if (remaining[i] == 0) {
                tasks[i].completion_time = elapsed;
                tasks[i].turnaround_time = elapsed;
                tasks[i].waiting_time =
                    elapsed - tasks[i].burst_time;
                tasks[i].state = TASK_TERMINATED;
                completed++;
            } else {
                tasks[i].state = TASK_READY;
            }
        }
    }

    print("\nID WAIT TAT CT");

    for (int i = 0; i < task_count; i++) {
        Task *t = &tasks[i];

        total_wait += t->waiting_time;
        total_tat += t->turnaround_time;

        print("\n");
        print_number(t->id);
        print("   ");
        print_number(t->waiting_time);
        print("   ");
        print_number(t->turnaround_time);
        print("   ");
        print_number(t->completion_time);
    }

    print("\nAverage waiting time: ");
    print_number(total_wait * 100 / task_count);
    print(".");
    print_number((total_wait * 100 / task_count) % 100);

    print("\nAverage turnaround time: ");
    print_number(total_tat * 100 / task_count);
    print(".");
    print_number((total_tat * 100 / task_count) % 100);
    print("\n");
}

void task_reset(void)
{
    for (int i = 0; i < task_count; i++) {
        tasks[i].state = TASK_READY;
        tasks[i].completion_time = 0;
        tasks[i].waiting_time = 0;
        tasks[i].turnaround_time = 0;
    }

    print("\nTask states and scheduling metrics reset.");
}
