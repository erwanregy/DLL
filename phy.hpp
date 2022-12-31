#pragma once
#include <stdint.h>

class DLL;

class PHY {
public:
    DLL* dll;
    PHY();
    void send(uint8_t* frame, uint8_t frame_length);
    void receive();
};