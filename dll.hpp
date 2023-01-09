#pragma once
#include "config.hpp"
#include <stdint.h>

// Virtual DLL for isolated layer testing
#define VIRTUAL_RECEIVER

// Random error inserting
// #define RANDOM_ERRORS
// #define ERROR_RARITY 10
// #define DROP_RARITY 10

// Debugging
// #define DEBUG_DLL
// #define DEBUG_MAC
// #define DEBUG_TEST
// #define DEBUG_RANDOM_ERRORS

// Printing options
// #define PRINT_FRAMES
// #define PRINT_STEPS
// #define PRINT_CRC
// #define PRINT_BYTE_STUFFING
// #define PRINT_ESC_AND_FLAG

#define FLAG 0x7D
#define ESC  0x7E
#define DEVICE_MAC_ADDRESS 0
#define MAX_PACKET_LENGTH 8
#define POLYNOMIAL 65521

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

#ifdef VIRTUAL_RECEIVER
    #include <stdlib.h>
    enum PACKET_LENGTH_OPTIONS {
        RANDOM = 0,
        FIXED
    };
    enum PACKET_DATA_OPTIONS {
        ALL = 0,
        ESC_ONLY,
        FLAG_ONLY,
        FLAG_AND_ESC_ONLY,
        SEQUENTIAL,
    };
    enum DESTINATION_MAC_ADDRESS_OPTIONS {
        DEVICE = 0,
        BROADCAST,
        WRONG
    };
#else
    class NetworkLayer;
#endif

class DLL {
private:
    Frame frame;
    // Sending
    void byte_stuff();
    uint16_t calculate_crc();
    uint8_t* stuffed_frame;
    uint8_t stuffed_frame_length;
    // Receiving
    bool check_crc();
    void de_byte_stuff();
    uint8_t* reconstructed_packet;
    uint8_t reconstructed_packet_length;
    bool error_in_split_packet_sequence;
    uint8_t expected_split_packet_num;
    uint8_t expected_last_split_packet_num;
public:
    DLL();
    void send(uint8_t* packet, uint8_t packet_length, uint8_t destination_address);
    void receive(uint8_t* frame, uint8_t frame_length);
    #ifdef VIRTUAL_RECEIVER
        uint8_t* received_packet;
        uint8_t received_packet_length;
        bool test(uint8_t max_packet_length, PACKET_LENGTH_OPTIONS, PACKET_DATA_OPTIONS, DESTINATION_MAC_ADDRESS_OPTIONS);
    #else
        // NET layer
        NetworkLayer* net;
        // MAC sub-layer
        class MAC {
        private:
            DLL& llc;
            uint8_t back_off_counter;
        public:
            MAC(DLL&);
            void send(uint8_t* frame, uint8_t frame_length);
            void tick();
            void receive();
        } mac;
    #endif
};

void print(Frame);
void print(uint8_t* buffer, uint8_t buffer_length);

#ifdef PRINT_ESC_AND_FLAG
    #undef put_hex
    #define put_hex(hex)\
    if (hex == FLAG) {\
        printf("FLAG");\
    } else if (hex == ESC) {\
        printf(" ESC");\
    } else {\
        printf("0x%02X", hex);\
    }
#endif

#ifdef PRINT_CRC
    #ifndef PRINT_STEPS
        #define PRINT_STEPS
    #endif
#endif

#ifdef PRINT_BYTE_STUFFING
    #ifndef PRINT_STEPS
        #define PRINT_STEPS
    #endif
#endif

#ifdef PRINT_STEPS
    #ifndef PRINT_FRAMES
        #define PRINT_FRAMES
    #endif
#endif

#ifdef PRINT_FRAMES
    #ifndef DEBUG_DLL
        #define DEBUG_DLL
    #endif
#endif

#ifdef DEBUG_MEM_ELABORATE
    #ifndef DEBUG_MEM
        #define DEBUG_MEM
    #endif
#endif