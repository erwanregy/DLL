#pragma once

// DLL TEST
#define DLL_TEST
// #define DEBUG_DLL_TEST
// #define VIRTUAL_DLL

// DLL DEBUGGING
#define DEBUG_DLL
// #define DEBUG_DLL_FRAMES
// #define DEBUG_DLL_STEPS

// PHY DEBUGGING
#define DEBUG_PHY
#define DEBUG_PHY_BYTES

// PRINT ESC AND FLAGS
#define PRINT_ESC_FLAG

// MEMORY DEBUGGING
// #define DEBUG_MEM
// #define DEBUG_MEM_ELABORATE

#ifdef WINDOWS
    #include <cstdio>
    #include <vector>
    #include <cstdint>
    static std::vector<uint8_t> wire;
    #define put_byte(byte) wire.push_back(byte)
    #define get_byte() wire.front(); wire.erase(wire.begin())
    #define get_ch() getchar()
    #define put_ch(ch) putchar(ch)
    #define put_str(str) printf(str)
    #ifndef PRINT_ESC_FLAG
        #define put_hex(hex) printf("0x%02X", hex)
    #else
        #include "dll.hpp"
        #define put_hex(hex)\
        if (hex == FLAG) {\
            printf("FLAG");\
        } else if (hex == ESC) {\
            printf(" ESC");\
        } else {\
            printf("0x%02X", hex);\
        }
    #endif
    #define put_uint8(uint8) printf("%u", uint8)
    #define put_uint16(uint16) printf("%u", uint16)
    #define _delay_us(us) (0)
    #define _delay_ms(ms) (0)
#else // AVR
    #include "uart.h"
    #include <util/delay.h>
#endif

// Define DEBUG_DLL if DEBUG_DLL_FRAMES or DEBUG_DLL_STEPS is defined
#ifdef DEBUG_DLL_STEPS
    #ifndef DEBUG_DLL_FRAMES
        #define DEBUG_DLL_FRAMES
    #endif
#endif
#ifdef DEBUG_DLL_FRAMES
    #ifndef DEBUG_DLL
        #define DEBUG_DLL
    #endif
#endif

// Define DEBUG_MEM if DEBUG_MEM_ELABORATE is defined
#ifdef DEBUG_MEM_ELABORATE
    #ifndef DEBUG_MEM
        #define DEBUG_MEM
    #endif
#endif