#pragma once
#include <stdint.h>
#include "config.hpp"

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

#ifdef DLL_TEST
    void print(Frame);
    void print(uint8_t* ptr, uint8_t len);
#endif

class DLL {
#ifdef DLL_TEST
    public:
#else
    private:
#endif
    Frame frame;
    uint8_t* stuffed_frame;
    uint8_t stuffed_frame_len;
    uint8_t* reconstructed_packet;
    uint8_t reconstructed_packet_len;
    bool error;
    void byte_stuff();
    void de_byte_stuff();
    void calc_crc();
    bool check_crc();
public:
    #ifdef DLL_TEST
        uint8_t** sent_frames;
        uint8_t* sent_frame_lens;
        uint8_t num_sent_frames;
        // Frame* received_frames;
        // uint8_t num_received_frames;
        uint8_t* received_packet;
        uint8_t received_packet_len;
    #endif
    DLL();
    void send(uint8_t* packet_ptr, uint8_t packet_len, uint8_t dest_addr);
    bool receive(uint8_t* frame_ptr, uint8_t frame_len);
};