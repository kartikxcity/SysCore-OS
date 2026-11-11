
#include <stdint.h>
#include <stddef.h>
#include "task.h"
#include "interrupt.h"
#include "memory.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static volatile uint16_t *const VGA =
    (volatile uint16_t *)0xB8000;

static int row = 0;
static int col = 0;
static uint8_t color = 0x0F;

void print(const char *s);
static void scroll_screen(void)
{
    for (int r = 0; r < VGA_HEIGHT - 1; r++) {
        for (int c = 0; c < VGA_WIDTH; c++) {
            VGA[r * VGA_WIDTH + c] =
                VGA[(r + 1) * VGA_WIDTH + c];
        }
    }

    /* Clear the last row. */
    for (int c = 0; c < VGA_WIDTH; c++) {
        VGA[(VGA_HEIGHT - 1) * VGA_WIDTH + c] =
            ((uint16_t)color << 8) | ' ';
    }

    row = VGA_HEIGHT - 1;
}
static void put_char(char c)
{
    if (c == '\n') {
        col = 0;
        row++;
    }
    else if (c == '\b') {
        if (col > 0) {
            col--;

            VGA[row * VGA_WIDTH + col] =
                ((uint16_t)color << 8) | ' ';
        }
    }
    else {
        VGA[row * VGA_WIDTH + col] =
            ((uint16_t)color << 8) | c;

        col++;
    }

    if (col >= VGA_WIDTH) {
        col = 0;
        row++;
    }

    if (row >= VGA_HEIGHT) {
        scroll_screen();
    }
}
void print(const char *s)
{
    while (*s) {
        put_char(*s++);
    }
}

static void clear_screen(void)
{
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA[i] = ((uint16_t)color << 8) | ' ';
    }

    row = 0;
    col = 0;
}

static uint8_t read_scancode(void)
{
    uint8_t scan;

    do {
        __asm__ volatile ("inb $0x64, %0" : "=a"(scan));
    } while (!(scan & 1));

    __asm__ volatile ("inb $0x60, %0" : "=a"(scan));
    return scan;
}

static char scancode_to_char(uint8_t sc)
{
    static const char keys[128] = {
        [2]='1',[3]='2',[4]='3',[5]='4',[6]='5',
        [7]='6',[8]='7',[9]='8',[10]='9',[11]='0',
        [12]='-',[13]='=',[14]='\b',[16]='q',[17]='w',
        [18]='e',[19]='r',[20]='t',[21]='y',[22]='u',
        [23]='i',[24]='o',[25]='p',[26]='[',[27]=']',
        [28]='\n',[30]='a',[31]='s',[32]='d',[33]='f',
        [34]='g',[35]='h',[36]='j',[37]='k',[38]='l',
        [39]=';',[40]='\'',[41]='`',[43]='\\',[44]='z',
        [45]='x',[46]='c',[47]='v',[48]='b',[49]='n',
        [50]='m',[51]=',',[52]='.',[53]='/',
        [57]=' '
    };

    if (sc < 128)
        return keys[sc];

    return 0;
}

static int equals(const char *a, const char *b)
{
    while (*a && *b) {
        if (*a != *b)
            return 0;
        a++;
        b++;
    }

    return *a == *b;
}

static char command[80];
static int command_pos = 0;

static void prompt(void)
{
    print("\nSysCore> ");
    command_pos = 0;
}


static void execute(const char *cmd)
{
    if (equals(cmd, "help")) {
print("\ncreate, tasks, taskmem, cleanup");
print("\nfcfs, sjf, priority, rr, reset");
    }
    else if (equals(cmd, "about")) {
        print("\nSysCore Mini OS");
        print("\nBootable C kernel running in QEMU.");
    }
    else if (equals(cmd, "clear")) {
        clear_screen();
    }
    else if (equals(cmd, "create")) {
        int id = task_create("Task", 3);
        if (id > 0)
            print("\nTask created successfully. ID: ");
        else
            print("\nTask creation failed.");
    }
    else if (equals(cmd, "tasks")) {
        task_list();
    }
    else if (equals(cmd, "fcfs")) {
        task_run_fcfs();
    }
    else if (equals(cmd, "sjf")) {
        task_run_sjf();
    }
    else if (equals(cmd, "priority")) {
        task_run_priority();
    }
    else if (equals(cmd, "rr")) {
        task_run_rr();
    }
    else if (equals(cmd, "reset")) {
        task_reset();
    }
    else if (equals(cmd, "schedule")) {
        task_run_fcfs();
    }
    else if (equals(cmd, "meminfo")) {
    memory_print_stats();
}
else if (equals(cmd, "memtest")) {
    memory_test();
}
    else if (equals(cmd, "halt")) {
        print("\nSysCore halted.");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
else if (equals(cmd, "taskmem")) {
    task_memory_list();
}
else if (equals(cmd, "cleanup")) {
    task_cleanup_memory();
}
    else {
        print("\nUnknown command. Type help.");
    }
}

void kernel_main(void)
{
    clear_screen();

    print("==============================");
    print("\n     SYSCORE MINI OS");
    print("\n==============================");
    print("\nType help to see commands.");

    task_init();
  /*  interrupts_init();*/
    memory_init();
    prompt();

    while (1) {
        uint8_t sc = read_scancode();

        if (sc & 0x80)
            continue;

        char c = scancode_to_char(sc);

        if (c == '\n') {
            command[command_pos] = '\0';
            execute(command);
            prompt();
        }
        else if (c == '\b') {
            if (command_pos > 0) {
                command_pos--;
                put_char('\b');
            }
        }
        else if (c && command_pos < 78) {
            command[command_pos++] = c;
            put_char(c);
        }
    }
}

