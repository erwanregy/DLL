#pragma once
#include <stdint.h>
#include "config.hpp"

#define FLAG 0x7D
#define ESC  0x7E
#define MAC_ADDRESS 0
#define MAX_PACKET_LENGTH 8
#define CRC_POLYNOMIAL 65521

// class PHY;

struct Frame {
    uint8_t header;
    uint8_t control[2];
    uint8_t addressing[2];
    uint8_t length;
    uint8_t* net_packet;
    uint8_t checksum[2];
    uint8_t footer;
    Frame();
};

class DLL {
private:
    Frame frame;
    uint8_t* stuffed_frame;
    uint8_t stuffed_frame_length;
    uint8_t* reconstructed_packet;
    uint8_t reconstructed_packet_length;
    void byte_stuff();
    void de_byte_stuff();
    uint16_t calculate_crc();
    bool check_crc();
    bool split_packet_error;
public:
    #ifndef DLL_TEST
        PHY* phy;
    #endif
    DLL(NetworkLayer* net);
    void send(uint8_t* packet, uint8_t packet_length, uint8_t destination_address);
    void receive(uint8_t* frame, uint8_t frame_length);
};

void print(Frame);
void print(uint8_t* pointer, uint8_t length);
