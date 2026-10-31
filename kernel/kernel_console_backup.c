
#include <stdint.h>
#include <stddef.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static volatile uint16_t *const VGA =
    (volatile uint16_t *)0xB8000;

static size_t row = 0;
static size_t col = 0;
static uint8_t color = 0x0F;

static void put_char(char c)
{
    if (c == '\n') {
        col = 0;
        row++;
    } else if (c == '\b') {
        if (col > 0) {
            col--;
            VGA[row * VGA_WIDTH + col] =
                ((uint16_t)color << 8) | ' ';
        }
    } else {
        VGA[row * VGA_WIDTH + col] =
            ((uint16_t)color << 8) | (uint8_t)c;
        col++;
    }

    if (col >= VGA_WIDTH) {
        col = 0;
        row++;
    }

    if (row >= VGA_HEIGHT) {
        row = 0;
    }
}

static void print(const char *s)
{
    while (*s) {
        put_char(*s++);
    }
}

static void clear_screen(void)
{
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA[i] = ((uint16_t)color << 8) | ' ';
    }
    row = 0;
    col = 0;
}

static uint8_t read_scancode(void)
{
    uint8_t code;

    __asm__ volatile (
        "1: inb $0x64, %%al\n"
        "testb $1, %%al\n"
        "jz 1b\n"
        "inb $0x60, %%al\n"
        : "=a"(code)
        :
        : "memory"
    );

    return code;
}

static char scancode_to_char(uint8_t sc)
{
    static const char keys[128] = {
        [0x02]='1', [0x03]='2', [0x04]='3',
        [0x05]='4', [0x06]='5', [0x07]='6',
        [0x08]='7', [0x09]='8', [0x0A]='9',
        [0x0B]='0', [0x10]='q', [0x11]='w',
        [0x12]='e', [0x13]='r', [0x14]='t',
        [0x15]='y', [0x16]='u', [0x17]='i',
        [0x18]='o', [0x19]='p', [0x1E]='a',
        [0x1F]='s', [0x20]='d', [0x21]='f',
        [0x22]='g', [0x23]='h', [0x24]='j',
        [0x25]='k', [0x26]='l', [0x2C]='z',
        [0x2D]='x', [0x2E]='c', [0x2F]='v',
        [0x30]='b', [0x31]='n', [0x32]='m',
        [0x39]=' ', [0x0C]='-', [0x0D]='='
    };

    if (sc < 128) {
        return keys[sc];
    }
    return 0;
}

static int equals(const char *a, const char *b)
{
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

static void prompt(void)
{
    print("\nSysCore> ");
}

static void execute(const char *cmd)
{
    if (equals(cmd, "help")) {
        print("\nCommands: help, about, clear, halt");
    } else if (equals(cmd, "about")) {
        print("\nSysCore Mini OS");
        print("\nBootable C kernel running in QEMU.");
    } else if (equals(cmd, "clear")) {
        clear_screen();
    } else if (equals(cmd, "halt")) {
        print("\nHalting CPU.");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    } else if (*cmd) {
        print("\nUnknown command. Type help.");
    }
}

void kernel_main(void)
{
    char input[64];
    size_t length = 0;

    clear_screen();
    print("================================");
    print("\n       SYSCORE MINI OS");
    print("\n================================");
    print("\nType help to see commands.");
    prompt();

    for (;;) {
        uint8_t sc = read_scancode();

        if (sc == 0xE0) {
            (void)read_scancode();
            continue;
        }

        if (sc & 0x80) {
            continue;
        }

        if (sc == 0x1C) {
            input[length] = '\0';
            execute(input);
            length = 0;
            prompt();
            continue;
        }

        if (sc == 0x0E) {
            if (length > 0) {
                length--;
                put_char('\b');
            }
            continue;
        }

        char c = scancode_to_char(sc);
        if (c && length < sizeof(input) - 1) {
            input[length++] = c;
            put_char(c);
        }
    }
}
