#pragma once
#include <stdint.h>

static uint16_t mem_use;

bool mem_leak();
void print_mem_use();

void allocate(uint8_t*& pointer, uint8_t& length, uint8_t new_length);
void allocate(uint8_t**& pointer, uint8_t*& lengths, uint8_t& length, uint8_t new_length);

void reallocate(uint8_t*& pointer, uint8_t& length, uint8_t new_length);
// void reallocate(uint8_t**& pointer_pointer, uint8_t*& length_pointer, uint8_t& length, uint8_t new_length);

void deallocate(uint8_t*& pointer, uint8_t& length);
void deallocate(uint8_t**& pointer_pointer, uint8_t*& length_pointer, uint8_t& length);