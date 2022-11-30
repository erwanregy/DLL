#include "mem.hpp"
#include <stdlib.h>
#include "config.hpp"

bool mem_leak() {
    if (mem_use != 0) {
        return 1;
    } else {
        return 0;
    }
}

void print_mem_use() {
    if (mem_use >= 1024) {
        put_uint16(mem_use/1024);  put_str(" KiB in use\r\n");
    } else {
        put_uint16(mem_use); put_str(" B in use\r\n");
    }
}

void allocate(uint8_t*& ptr, uint8_t& len, uint8_t new_len) {
    ptr = (uint8_t*) malloc(sizeof(*ptr) * new_len);
    if (ptr == NULL) {
        #ifdef DEBUG_MEM
            put_str("Failed to allocate\r\n");
        #endif
        return;
    }
    uint8_t num_bytes = sizeof(*ptr) * new_len;
    mem_use += num_bytes;
    #ifdef DEBUG_MEM
        put_str("Allocated "); put_uint8(num_bytes); put_str(" bytes (+"); put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    len = new_len;
}

void allocate(uint8_t**& ptr, uint8_t*& lens, uint8_t& len, uint8_t new_len) {
    ptr = (uint8_t**) malloc(sizeof(*ptr) * new_len);
    if (ptr == NULL) {
        #ifdef DEBUG_MEM
            put_str("Failed to allocate\r\n");
        #endif
        return;
    }
    uint8_t num_bytes = sizeof(*ptr) * new_len;
    mem_use += num_bytes;
    #ifdef DEBUG_MEM
        put_str("Allocated "); put_uint8(num_bytes); put_str(" bytes (+"); put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    allocate(lens, len, new_len);
}

void reallocate(uint8_t*& ptr, uint8_t& len, uint8_t new_len) {
    ptr = (uint8_t*) realloc(ptr, sizeof(*ptr) * new_len);
    if (ptr == NULL) {
        #ifdef DEBUG_MEM
            put_str("Failed to reallocate\r\n");
        #endif
        return;
    }
    int8_t num_bytes = sizeof(*ptr) * (new_len - len);
    mem_use += num_bytes;
    #ifdef DEBUG_MEM
        put_str("Reallocated from "); put_uint8(sizeof(*ptr) * len); put_str(" to "); put_uint8(sizeof(*ptr) * new_len); put_str(" bytes (");
        if (num_bytes & (1 << 7)) {
            put_xh('-');
            num_bytes = sizeof(*ptr) * (len - new_len);
        } else {
            put_ch('+');
        }
        put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    len = new_len;
}

// void reallocate(uint8_t**& ptr, uint8_t*& lens, uint8_t& len, uint8_t new_len) {
//     ptr = (uint8_t**) realloc(ptr, sizeof(*ptr) * new_len);
//     if (ptr == NULL) {
//         #ifdef DEBUG_MEM
//             put_str("Failed to reallocate\r\n");
//         #endif
//         return;
//     }
//     int8_t num_bytes = sizeof(*ptr) * (new_len - len);
//     mem_use += num_bytes;
//     #ifdef DEBUG_MEM
//         put_str("Reallocated from "); put_uint8(sizeof(*ptr) * len); put_str(" to "); put_uint8(sizeof(*ptr) * new_len); put_str(" bytes (");
//         if (num_bytes & (1 << 7)) {
//             put_ch('-');
//             num_bytes = sizeof(*ptr) * (len - new_len);
//         } else {
//             put_ch('+');
//         }
//         put_uint8(num_bytes); put_str(")\r\n");
//         print_mem_use();
//     #endif
//     reallocate(lens, len, new_len);
// }


void deallocate(uint8_t*& ptr, uint8_t& len) {
    if (ptr == NULL) {
        #ifdef DEBUG_MEM
            put_str("Cannot deallocate: already deallocated\r\n");
        #endif
        len = 0;
        return;
    }
    free(ptr);
    uint8_t num_bytes = sizeof(*ptr) * len;
    mem_use -= num_bytes;
    #ifdef DEBUG_MEM
        put_str("Deallocated "); put_uint8(num_bytes); put_str(" bytes (-"); put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    ptr = NULL;
    len = 0;
}

void deallocate(uint8_t**& ptr, uint8_t*& lens, uint8_t& len) {
    if (ptr == NULL) {
        #ifdef DEBUG_MEM
            put_str("Cannot deallocate: already deallocated\r\n");
        #endif
        len = 0;
        return;
    }
    free(ptr);
    uint8_t num_bytes = sizeof(*ptr) * len;
    mem_use -= num_bytes;
    #ifdef DEBUG_MEM
        put_str("Deallocated "); put_uint8(num_bytes); put_str(" bytes (-"); put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    ptr = NULL;
    deallocate(lens, len);
}