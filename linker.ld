
ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}

SECTIONS
{
    . = 1M;

    .multiboot :
    {
        KEEP(*(.multiboot))
    } :text

    . = ALIGN(4K);

    .text :
    {
        *(.text*)
    } :text

    .rodata :
    {
        *(.rodata*)
    } :text

    . = ALIGN(4K);

    .data :
    {
        *(.data*)
    } :data

    .bss :
    {
        *(COMMON)
        *(.bss*)
    } :data

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note*)
    }
}
