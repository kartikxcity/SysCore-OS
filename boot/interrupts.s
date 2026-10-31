.section .text
.global timer_interrupt_stub
.extern timer_handler_c

timer_interrupt_stub:
    pusha
    cld
    call timer_handler_c
    popa
    iret
