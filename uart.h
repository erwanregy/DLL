#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>

#ifndef UART_H
#define UART_H
#define F_CPU 12000000

//uart
void init_uart0(void);
char get_ch(void);
void put_ch(char ch);
void put_str(const char* str);
void put_hex(uint8_t byte);
void put_uint8(uint8_t num);
void put_uint16(uint16_t num);

#endif
