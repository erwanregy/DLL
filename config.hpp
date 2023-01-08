#pragma once
#include "dll.hpp"

// Virtual DLL Test
#define VIRTUAL_DLL
// #define DEBUG_TEST
// #define RANDOM_ERRORS

// DLL Debugging
// #define DEBUG_DLL

// Printing
// #define PRINT_FRAMES
// #define PRINT_STEPS
// #define PRINT_BYTE_STUFFING
// #define PRINT_ESC_FLAG

// Memory debugging
#define DEBUG_MEM
// #define DEBUG_MEM_ELABORATE

#ifdef WINDOWS
    #include <stdio.h>
    #define put_ch(ch) putchar(ch)
    #define put_str(str) printf(str)
    #ifdef PRINT_ESC_FLAG
        #define put_hex(hex)\
        if (hex == FLAG) {\
            printf("FLAG");\
        } else if (hex == ESC) {\
            printf(" ESC");\
        } else {\
            printf("0x%02X", hex);\
        }
    #else
        #define put_hex(hex) printf("0x%02X", hex)
    #endif
    #define put_uint8(uint8) printf("%u", uint8)
    #define put_uint16(uint16) printf("%u", uint16)
#else // AVR
    #include "uart.h"
    #include <util/delay.h>
#endif

#ifdef PRINT_BYTE_STUFFING
    #ifndef PRINT_STEPS
        #define PRINT_STEPS
    #endif
#endif

#ifdef PRINT_STEPS
    #ifndef PRINT_FRAMES
        #define PRINT_FRAMES
    #endif
#endif

#ifdef PRINT_FRAMES
    #ifndef DEBUG_DLL
        #define DEBUG_DLL
    #endif
#endif

#ifdef DEBUG_MEM_ELABORATE
    #ifndef DEBUG_MEM
        #define DEBUG_MEM
    #endif
#endif