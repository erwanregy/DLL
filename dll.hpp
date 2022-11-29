#pragma once
#include <stdint.h>
#include "debug.hpp"

#define FLAG     0x7D
#define ESC      0x7E
#define MAC_ADDR 0

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

struct Buffer {
    uint8_t* ptr;
    uint8_t length;
    Buffer();
};

Buffer byte_stuff(Frame);
Frame de_byte_stuff(Buffer);

class DLL {
#ifdef DEBUG
    public:
#else
    private:
#endif
    Frame frame;
    Buffer stuffed_frame;
    #ifdef DEBUG
        Buffer* sent_frames;
        uint8_t num_sent_frames;
        Buffer received_packet;
    #endif
    Buffer reconstructed_packet;
    bool error;
    void calc_crc();
    bool check_crc();
public:
    DLL();
    void send(uint8_t* packet_ptr, uint8_t packet_length, uint8_t dest_addr);
    bool receive(uint8_t* frame_ptr, uint8_t frame_length);
};

#ifdef DEBUG
    void print(Frame);
    void print(Buffer);
    void print(uint8_t* ptr, uint8_t length);
#endif