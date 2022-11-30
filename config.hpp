#pragma once

#define DLL_TEST
// #define DEBUG_DLL
// #define DEBUG_MEM
// #define SILENCE

#ifdef SILENCE
    #define put_ch(ch) (0)
    #define put_str(str) (0)
    #define put_hex(hex) (0)
    #define put_uint8(uint8) (0)
    #define put_uint16(uint16) (0)
    #define init_uart0() (0)
    #define _delay_ms(ms) (0)
#else
    #ifdef WINDOWS
        #include <stdio.h>
        #define put_ch(ch) putchar(ch)
        #define put_str(str) printf(str)
        #define put_hex(hex) printf("0x%02X", hex)
        #define put_uint8(uint8) printf("%u", uint8)
        #define put_uint16(uint16) printf("%u", uint16)
        #define init_uart0() (0)
        #define _delay_ms(ms) (0)
    #else
        #include "uart.h"
        #include <util/delay.h>
    #endif
#endif