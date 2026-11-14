#ifndef SERIAL_H
#define SERIAL_H

void serial_init(void);
void serial_write(char c);
void serial_print(const char *str);
void serial_print_number(int num);

#endif
