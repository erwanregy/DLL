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

void mem_test() {
    #define NUM_PTRS 24
    #define MAX_LEN 128
    #define NUM_ITERATIONS 3
    uint8_t* ptr[NUM_PTRS];
    uint8_t length[NUM_PTRS];
    for (uint8_t i = 0; i < NUM_PTRS; i++) {
        ptr[i] = NULL;
        length[i] = 0;
    }
    for (uint8_t i = 0; i < NUM_PTRS; i++) {
        allocate(ptr[i], length[i], (uint8_t) 128);
    }
    put_str("\r\n");
    for (uint8_t i = 0; i < NUM_ITERATIONS; i++) {
        for (uint8_t i = 0; i < NUM_PTRS; i++) {
            reallocate(ptr[i], length[i], (uint8_t) (rand() % MAX_LEN));
        }
        put_str("\r\n");
    }
    for (uint8_t i = 0; i < NUM_PTRS; i++) {
        deallocate(ptr[i], length[i]);
    }
}

void print_mem_use() {
    if (mem_use >= 1024) {
        put_uint16(mem_use/1024);  put_str(" KiB in use\r\n");
    } else {
        put_uint16(mem_use); put_str(" B in use\r\n");
    }
}


void allocate(uint8_t*& ptr, uint8_t& ptr_length, uint8_t new_length) {
    ptr = (uint8_t*) malloc(sizeof(*ptr) * new_length);
    if (ptr == NULL) {
        #ifdef DEBUG_MEM
            put_str("Failed to allocate\r\n");
        #endif
        return;
    }
    uint8_t num_bytes = sizeof(*ptr) * new_length;
    mem_use += num_bytes;
    #ifdef DEBUG_MEM
        put_str("Allocated "); put_uint8(num_bytes); put_str(" bytes (+"); put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    ptr_length = new_length;
}

void allocate(Buffer*& ptr, uint8_t& ptr_length, uint8_t new_length) {
    ptr = (Buffer*) malloc(sizeof(*ptr) * new_length);
    if (ptr == NULL) {
        #ifdef DEBUG_MEM
            put_str("Failed to allocate\r\n");
        #endif
        return;
    }
    uint8_t num_bytes = sizeof(*ptr) * new_length;
    mem_use += num_bytes;
    #ifdef DEBUG_MEM
        put_str("Allocated "); put_uint8(num_bytes); put_str(" bytes (+"); put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    ptr_length = new_length;
}

void allocate(Buffer& buffer, uint8_t new_length) {
    buffer.ptr = (uint8_t*) malloc(sizeof(*buffer.ptr) * new_length);
    if (buffer.ptr == NULL) {
        #ifdef DEBUG_MEM
            put_str("Failed to allocate\r\n");
        #endif
        return;
    }
    uint8_t num_bytes = sizeof(*buffer.ptr) * new_length;
    mem_use += num_bytes;
    #ifdef DEBUG_MEM
        put_str("Allocated "); put_uint8(num_bytes); put_str(" bytes (+"); put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    buffer.length = new_length;
}


void reallocate(uint8_t*& ptr, uint8_t& ptr_length, uint8_t new_length) {
    ptr = (uint8_t*) realloc(ptr, sizeof(*ptr) * new_length);
    if (ptr == NULL) {
        #ifdef DEBUG_MEM
            put_str("Failed to reallocate\r\n");
        #endif
        return;
    }
    int8_t num_bytes = sizeof(*ptr) * (new_length - ptr_length);
    mem_use += num_bytes;
    #ifdef DEBUG_MEM
        put_str("Reallocated from "); put_uint8(sizeof(*ptr) * ptr_length); put_str(" to "); put_uint8(sizeof(*ptr) * new_length); put_str(" bytes (");
        if (num_bytes & (1 << 7)) {
            put_str("-");
            num_bytes = sizeof(*ptr) * (ptr_length - new_length);
        } else {
            put_str("+");
        }
        put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    ptr_length = new_length;
}

void reallocate(Buffer*& ptr, uint8_t& ptr_length, uint8_t new_length) {
    ptr = (Buffer*) realloc(ptr, sizeof(*ptr) * new_length);
    if (ptr == NULL) {
        #ifdef DEBUG_MEM
            put_str("Failed to reallocate\r\n");
        #endif
        return;
    }
    int8_t num_bytes = sizeof(*ptr) * (new_length - ptr_length);
    mem_use += num_bytes;
    #ifdef DEBUG_MEM
        put_str("Reallocated from "); put_uint8(sizeof(*ptr) * ptr_length); put_str(" to "); put_uint8(sizeof(*ptr) * new_length); put_str(" bytes (");
        if (num_bytes & (1 << 7)) {
            put_str("-");
            num_bytes = sizeof(*ptr) * (ptr_length - new_length);
        } else {
            put_str("+");
        }
        put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    ptr_length = new_length;
}

void reallocate(Buffer& buffer, uint8_t new_length) {
    buffer.ptr = (uint8_t*) realloc(buffer.ptr, sizeof(*buffer.ptr) * new_length);
    if (buffer.ptr == NULL) {
        #ifdef DEBUG_MEM
            put_str("Failed to reallocate\r\n");
        #endif
        return;
    }
    int8_t num_bytes = sizeof(*buffer.ptr) * (new_length - buffer.length);
    mem_use += num_bytes;
    #ifdef DEBUG_MEM
        put_str("Reallocated from "); put_uint8(sizeof(*buffer.ptr) * buffer.length); put_str(" to "); put_uint8(sizeof(*buffer.ptr) * (new_length)); put_str(" bytes (");
        if (num_bytes >> 7 == 1) {
            put_str("-");
            num_bytes = sizeof(*buffer.ptr) * (buffer.length - new_length);
        } else {
            put_str("+");
        }
        put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    buffer.length = new_length;
}


void deallocate(uint8_t*& ptr, uint8_t& ptr_length) {
    if (ptr == NULL) {
        #ifdef DEBUG_MEM
            put_str("Cannot deallocate: already deallocated\r\n");
        #endif
        ptr_length = 0;
        return;
    }
    free(ptr);
    uint8_t num_bytes = sizeof(*ptr) * ptr_length;
    mem_use -= num_bytes;
    #ifdef DEBUG_MEM
        put_str("Deallocated "); put_uint8(num_bytes); put_str(" bytes (-"); put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    ptr = NULL;
    ptr_length = 0;
}

void deallocate(Buffer*& ptr, uint8_t& ptr_length) {
    if (ptr == NULL) {
        #ifdef DEBUG_MEM
            put_str("Cannot deallocate: already deallocated\r\n");
        #endif
        ptr_length = 0;
        return;
    }
    free(ptr);
    uint8_t num_bytes = sizeof(*ptr) * ptr_length;
    mem_use -= num_bytes;
    #ifdef DEBUG_MEM
        put_str("Deallocated "); put_uint8(num_bytes); put_str(" bytes (-"); put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    ptr = NULL;
    ptr_length = 0;
}

void deallocate(Buffer& buffer) {
    if (buffer.ptr == NULL) {
        #ifdef DEBUG_MEM
            put_str("Cannot deallocate: already deallocated\r\n");
        #endif
        buffer.length = 0;
        return;
    }
    free(buffer.ptr);
    uint8_t num_bytes = sizeof(*buffer.ptr) * buffer.length;
    mem_use -= num_bytes;
    #ifdef DEBUG_MEM
        put_str("Deallocated "); put_uint8(sizeof(*buffer.ptr) * buffer.length); put_str(" bytes (-"); put_uint8(num_bytes); put_str(")\r\n");
        print_mem_use();
    #endif
    buffer.ptr = NULL;
    buffer.length = 0;
}