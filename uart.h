#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>

#ifndef UART_H
#define UART_H
#define F_CPU 12000000

//uart
void init_uart0();
void init_uart1();
uint8_t receive_byte();
void send_byte(uint8_t);
char get_ch();
void put_ch(char);
void put_str(const char*);
void put_hex(uint8_t);
void put_uint8(uint8_t);
void put_uint16(uint16_t);

#endif
