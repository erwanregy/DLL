#pragma once
#include "dll.hpp"

// VIRTUAL DLL TEST
#define DLL_TEST
#define DEBUG_DLL_TEST

// DLL DEBUGGING
#define DEBUG_DLL
#define DEBUG_DLL_FRAMES
// #define DEBUG_DLL_STEPS

// PRINT ESC AND FLAGS
#define PRINT_ESC_FLAG

// MEMORY DEBUGGING
// #define DEBUG_MEM
// #define DEBUG_MEM_ELABORATE

#ifdef WINDOWS
    #include <stdio.h>
    #define put_ch(ch) putchar(ch)
    #define put_str(str) printf(str)
    #ifndef PRINT_ESC_FLAG
        #define put_hex(hex) printf("0x%02X", hex)
    #else
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
#else // AVR
    #include "uart.h"
    #include <util/delay.h>
#endif

// Define DEBUG_DLL if DEBUG_DLL_FRAMES or DEBUG_DLL_STEPS is defined
#ifdef DEBUG_DLL_FRAMES
    #ifndef DEBUG_DLL
        #define DEBUG_DLL
    #endif
#endif
#ifdef DEBUG_DLL_STEPS
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