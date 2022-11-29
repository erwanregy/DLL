#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint8_t header = 0b01111110;
    uint16_t control;
    uint16_t addressing;
    uint8_t length;
    uint8_t* packet;
    uint16_t checksum;
    uint8_t footer = 0b01111110;
} FRAME;

FRAME* init_frame(uint8_t length) {
    // Allocate memory to frame with length of packet accounted for
    FRAME* frame = (FRAME*) malloc(sizeof(FRAME) - sizeof(FRAME::packet) + sizeof(uint8_t)*length);
    frame->length = length;
    return frame;
}

uint16_t calc_crc(FRAME frame) {
    uint8_t* frame_data = (uint8_t*) malloc(sizeof(frame) - sizeof(frame.header) - sizeof(frame.footer));

    uint8_t i = 0;
    for (uint8_t* byte_ptr = &frame.header; byte_ptr < (uint8_t*) &frame.checksum; byte_ptr++) {
        uint8_t byte = *byte_ptr;
        frame_data[i] = byte;
        i++;
    }
    // Add 0's to last 2 bytes (last 16 bits) for 16-bit (2-byte) CRC
    frame_data[sizeof(frame_data)/sizeof(uint8_t) - 1] = 0;
    frame_data[sizeof(frame_data)/sizeof(uint8_t)] = 0;

    for (uint8_t i = 0; i < sizeof(frame_data)/sizeof(uint8_t); i++) {
        // do CRC
    }

    return crc;
}

void byte_stuff(FRAME& frame) {
    // Iterate over entire frame excluding header and footer
    for (uint8_t* byte_ptr = (uint8_t*)&frame.header; byte_ptr < &frame.footer; byte_ptr++) {
        uint8_t& byte = *byte_ptr;
        // If flag or escape character are encountered
        if (byte == 0b01111110 || byte == 0b00010000) {
            // Shift all bytes forward by one
            for (uint8_t* temp_ptr = (uint8_t*)&frame.footer + 1; temp_ptr > byte_ptr; temp_ptr--) {
                uint8_t& current_byte = *temp_ptr;
                uint8_t prev_byte = *(temp_ptr - 1);
                current_byte = prev_byte;
            }
            // Write escape character into current byte
            byte = 0b00010000;
            // Skip a byte (the one we just encountered but moved forward)
            byte_ptr++;
        } 
    }
}

void send() {
    FRAME frame;
    PACKET packet;

    while (true) {
        from_net(&packet);
        frame.packet = packet;
        to_phy(&frame);
    }
}

void receive() {
    EVENT event;
    FRAME frame;
    PACKET packet;

    while (true) {
        wait_for_event(&event);
        from_phy(&frame);
        packet = frame.packet;
        to_net(&packet);
    }
}