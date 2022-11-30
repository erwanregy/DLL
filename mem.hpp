#pragma once
#include <stdint.h>

static uint16_t mem_use;

bool mem_leak();
void print_mem_use();

void allocate(uint8_t*& ptr, uint8_t& len, uint8_t new_len);
void allocate(uint8_t**& ptr, uint8_t*& lens, uint8_t& len, uint8_t new_len);

void reallocate(uint8_t*& ptr, uint8_t& len, uint8_t new_len);
// void reallocate(uint8_t**& ptr, uint8_t*& lens, uint8_t& len, uint8_t new_len);

void deallocate(uint8_t*& ptr, uint8_t& len);
void deallocate(uint8_t**& ptr, uint8_t*& lens, uint8_t& len);