#include "mem.hpp"
#include <stdlib.h>
#include <string.h>
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

void allocate(uint8_t*& buffer, uint8_t& length, uint8_t new_length) {
    buffer = (uint8_t*) malloc(sizeof(*buffer) * new_length);
    if (buffer == NULL) {
        #ifdef DEBUG_MEM
            put_str("Failed to allocate\r\n");
        #endif
        return;
    }
    uint8_t num_bytes = sizeof(*buffer) * new_length;
    mem_use += num_bytes;
    #ifdef DEBUG_MEM
        put_str("Allocated "); put_uint8(num_bytes); put_str(" bytes (+"); put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    length = new_length;
}

void allocate(uint8_t**& buffer, uint8_t*& lengths, uint8_t& length, uint8_t new_length) {
    buffer = (uint8_t**) malloc(sizeof(*buffer) * new_length);
    if (buffer == NULL) {
        #ifdef DEBUG_MEM
            put_str("Failed to allocate\r\n");
        #endif
        return;
    }
    uint8_t num_bytes = sizeof(*buffer) * new_length;
    mem_use += num_bytes;
    #ifdef DEBUG_MEM
        put_str("Allocated "); put_uint8(num_bytes); put_str(" bytes (+"); put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    allocate(lengths, length, new_length);
}

void reallocate(uint8_t*& buffer, uint8_t& length, uint8_t new_length) {
    uint8_t temp[new_length];
    memcpy(temp, buffer, new_length);
    buffer = (uint8_t*) malloc(sizeof(*buffer) * new_length);
    memcpy(buffer, temp, new_length);
    if (buffer == NULL) {
        #ifdef DEBUG_MEM
            put_str("Failed to reallocate\r\n");
        #endif
        return;
    }
    int8_t num_bytes = sizeof(*buffer) * (new_length - length);
    mem_use += num_bytes;
    #ifdef DEBUG_MEM
        put_str("Reallocated from "); put_uint8(sizeof(*buffer) * length); put_str(" to "); put_uint8(sizeof(*buffer) * new_length); put_str(" bytes (");
        if (num_bytes & (1 << 7)) {
            put_ch('-');
            num_bytes = sizeof(*buffer) * (length - new_length);
        } else {
            put_ch('+');
        }
        put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    length = new_length;
}

// void reallocate(uint8_t**& buffer, uint8_t*& lengths, uint8_t& length, uint8_t new_length) {
//     uint8_t* temp[new_length];
//     memcpy(temp, buffer, new_length);
//     buffer = (uint8_t**) malloc(sizeof(*buffer) * new_length);
//     memcpy(buffer, temp, new_length);
//     int8_t num_bytes = sizeof(*buffer) * (new_length - length);
//     mem_use += num_bytes;
//     #ifdef DEBUG_MEM
//         put_str("Reallocated from "); put_uint8(sizeof(*buffer) * length); put_str(" to "); put_uint8(sizeof(*buffer) * new_length); put_str(" bytes (");
//         if (num_bytes & (1 << 7)) {
//             put_ch('-');
//             num_bytes = sizeof(*buffer) * (length - new_length);
//         } else {
//             put_ch('+');
//         }
//         put_uint8(num_bytes); put_str(")\r\n");
//         print_mem_use();
//     #endif
//     reallocate(lengths, length, new_length);
// }


void deallocate(uint8_t*& buffer, uint8_t& length) {
    if (buffer == NULL) {
        #ifdef DEBUG_MEM
            put_str("Cannot deallocate: already deallocated\r\n");
        #endif
        length = 0;
        return;
    }
    free(buffer);
    uint8_t num_bytes = sizeof(*buffer) * length;
    mem_use -= num_bytes;
    #ifdef DEBUG_MEM
        put_str("Deallocated "); put_uint8(num_bytes); put_str(" bytes (-"); put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    buffer = NULL;
    length = 0;
}

void deallocate(uint8_t**& buffer, uint8_t*& lengths, uint8_t& length) {
    if (buffer == NULL) {
        #ifdef DEBUG_MEM
            put_str("Cannot deallocate: already deallocated\r\n");
        #endif
        length = 0;
        return;
    }
    free(buffer);
    uint8_t num_bytes = sizeof(*buffer) * length;
    mem_use -= num_bytes;
    #ifdef DEBUG_MEM
        put_str("Deallocated "); put_uint8(num_bytes); put_str(" bytes (-"); put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    buffer = NULL;
    deallocate(lengths, length);
}