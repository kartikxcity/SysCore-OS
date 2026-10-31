#include <stdint.h>
#include "interrupt.h"

#define IDT_ENTRIES 256
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1
#define PIT_COMMAND  0x43
#define PIT_CHANNEL0 0x40

typedef struct __attribute__((packed)) {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t offset_high;
} IDTEntry;

typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint32_t base;
} IDTPointer;

extern void timer_interrupt_stub(void);

static IDTEntry idt[IDT_ENTRIES];
static volatile uint32_t ticks = 0;

static void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void idt_set_gate(uint8_t vector, uint32_t handler)
{
    idt[vector].offset_low = handler & 0xFFFF;
    idt[vector].selector = 0x08;
    idt[vector].zero = 0;
    idt[vector].flags = 0x8E;
    idt[vector].offset_high = (handler >> 16) & 0xFFFF;
}

static void pic_remap(void)
{
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    /* Keep the keyboard and timer IRQs enabled. */
    outb(PIC1_DATA, 0xFC);
    outb(PIC2_DATA, 0xFF);
}

void timer_handler_c(void)
{
    ticks++;
    outb(PIC1_COMMAND, 0x20);
}

uint32_t timer_get_ticks(void)
{
    return ticks;
}

void interrupts_init(void)
{
    __asm__ volatile ("cli");

    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt[i].offset_low = 0;
        idt[i].selector = 0x08;
        idt[i].zero = 0;
        idt[i].flags = 0x8E;
        idt[i].offset_high = 0;
    }

    idt_set_gate(32, (uint32_t)timer_interrupt_stub);

    IDTPointer idtr = {
        .limit = sizeof(idt) - 1,
        .base = (uint32_t)idt
    };

    __asm__ volatile ("lidt %0" : : "m"(idtr));

    pic_remap();

    /* PIT frequency: approximately 100 Hz. */
    uint16_t divisor = 11932;
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, divisor >> 8);

    __asm__ volatile ("sti");
}
