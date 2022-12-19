#include "uart.h"
#include "dll.hpp"

void init_uart0(void) {
	/* Configure 9600 baud , 8-bit , no parity and one stop bit */
	const int baud_rate = 9600;
	UBRR0H = (F_CPU/(baud_rate*16L)-1) >> 8;
	UBRR0L = (F_CPU/(baud_rate*16L)-1);
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);
	UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

char get_ch(void) {
	while (!(UCSR0A & _BV(RXC0)));
	return UDR0;
}

void put_ch(char ch) {
	while (!(UCSR0A & _BV(UDRE0)));
	UDR0 = ch;
}

void put_str(const char* str) {
	int i;
	for (i = 0; str[i]; i++) {
		put_ch(str[i]);
	}
}

void put_hex(uint8_t byte) {
	#ifdef PRINT_ESC_FLAG
		if (byte == FLAG) {
			put_str("FLAG");
			return;
		} else if (byte == ESC) {
			put_str(" ESC");
			return;
		}
	#endif
	put_str("0x");
	uint8_t nibble[2];
	nibble[0] = (byte & 0xF0) >> 4;
	nibble[1] = (byte & 0x0F);
	if (nibble[0] < 0xA){
		put_ch(nibble[0] + 48);
	} else {
		put_ch(nibble[0] + 55);
	}
	if (nibble[1] < 0xA){
		put_ch(nibble[1] + 48);
	} else {
		put_ch(nibble[1] + 55);
	}
}

void put_uint8(uint8_t num) {
	if (num < 10) {
		put_ch(num + '0');
	} else if (num < 100) {
		put_ch(num/10 + '0');
		put_ch(num%10 + '0');
	} else if (num < 1000) {
		put_ch(num/100 + '0');
		put_ch(num%100/10 + '0');
		put_ch(num%10 + '0');
	}
} 

void put_uint16(uint16_t num) {
	if (num < 10) {
		put_ch(num + '0');
	} else if (num < 100) {
		put_ch(num/10 + '0');
		put_ch(num%10 + '0');
	} else if (num < 1000) {
		put_ch(num/100 + '0');
		put_ch(num%100/10 + '0');
		put_ch(num%10 + '0');
	} else if (num < 10000) {
		put_ch(num/1000 + '0');
		put_ch(num%1000/100 + '0');
		put_ch(num%100/10 + '0');
		put_ch(num%10/1 + '0');
	} else {
		put_ch(num/10000 + '0');
		put_ch(num%10000/1000 + '0');
		put_ch(num%1000/100 + '0');
		put_ch(num%100/10 + '0');
		put_ch(num%10/1 + '0');
	}
}