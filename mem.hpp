#pragma once
#include <stdint.h>
#include "dll.hpp"

static uint16_t mem_use;

bool mem_leak();
void print_mem_use();

void allocate(uint8_t*& ptr, uint8_t& ptr_length, uint8_t new_length);
void allocate(Buffer*& ptr, uint8_t& ptr_length, uint8_t new_length);
void allocate(Buffer& buffer, uint8_t new_length);

void reallocate(uint8_t*& ptr, uint8_t& ptr_length, uint8_t new_length);
void reallocate(Buffer*& ptr, uint8_t& ptr_length, uint8_t new_length);
void reallocate(Buffer& buffer, uint8_t new_length);

void deallocate(uint8_t*& ptr, uint8_t& ptr_length);
void deallocate(Buffer*& ptr, uint8_t& ptr_length);
void deallocate(Buffer& buffer);

void mem_test();
