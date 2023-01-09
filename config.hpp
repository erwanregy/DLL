#pragma once

#ifdef WINDOWS
    #include <stdio.h>
    #define put_ch(ch) putchar(ch)
    #define put_str(str) printf(str)
    #define put_hex(hex) printf("0x%02X", hex)
    #define put_uint8(uint8) printf("%u", uint8)
    #define put_uint16(uint16) printf("%u", uint16)
#else // AVR
    #include "uart.h"
    #include <util/delay.h>
#endif