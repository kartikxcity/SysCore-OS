#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

void interrupts_init(void);
uint32_t timer_get_ticks(void);

#endif
