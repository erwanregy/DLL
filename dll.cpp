#include "dll.hpp"
#include <string.h>
#include "mem.hpp"

#define allocate(x, ...) put_str(#x); put_str(": "); allocate(x, ##__VA_ARGS__)
#define reallocate(x, ...) put_str(#x); put_str(": "); reallocate(x,##__VA_ARGS__)
#define deallocate(x, ...) put_str(#x); put_str(": "); deallocate(x, ##__VA_ARGS__)

void DLL::send(uint8_t* packet_ptr, uint8_t packet_length, uint8_t dest_addr) {
    if (packet_length == 0) {
        return;
    }
    #define MAX_PACKET_LENGTH 8
    bool remainder_packet = packet_length % MAX_PACKET_LENGTH;
    uint8_t num_split_packets = packet_length/MAX_PACKET_LENGTH + remainder_packet;
    #ifdef DEBUG
        allocate(sent_frames, num_sent_frames, num_split_packets);
    #endif
    for (uint8_t split_packet_num = 0; split_packet_num < num_split_packets; split_packet_num++) {
        uint8_t split_packet_length;
        if (split_packet_num == (num_split_packets - 1)) {
            split_packet_length = packet_length - (num_split_packets - 1) * MAX_PACKET_LENGTH;
        } else {
            split_packet_length = MAX_PACKET_LENGTH;
        }
        frame.control[0] = split_packet_num;
        frame.control[1] = num_split_packets - 1;
        frame.addressing[0] = MAC_ADDR;
        frame.addressing[1] = dest_addr;
        allocate(frame.net_packet, frame.length, split_packet_length);
        for (uint8_t i = 0; i < split_packet_length; i++) {
            frame.net_packet[i] = packet_ptr[split_packet_num * MAX_PACKET_LENGTH + i]; 
        }
        calc_crc();
        stuffed_frame = byte_stuff(frame); // allocates memory
        // PHY.send(stuffed_frame.ptr, stuffed_frame.length);
        #ifdef DEBUG
            // print(frame);
            // put_str("Stuffed frame: "); print(stuffed_frame);
            allocate(sent_frames[split_packet_num].ptr, sent_frames[split_packet_num].length, stuffed_frame.length);
            memcpy(sent_frames[split_packet_num].ptr, stuffed_frame.ptr, stuffed_frame.length);
        #endif
        deallocate(frame.net_packet, frame.length);
        deallocate(stuffed_frame);
    }
}

bool DLL::receive(uint8_t* frame_ptr, uint8_t frame_length) {
    allocate(stuffed_frame, frame_length);
    memcpy(stuffed_frame.ptr, frame_ptr, stuffed_frame.length);

    frame = de_byte_stuff(stuffed_frame);
    #ifdef DEBUG
        // put_str("Stuffed frame: "); print(stuffed_frame);
        // print(frame);
    #endif
    deallocate(stuffed_frame);

    // Check that destination MAC address in frame matches local MAC address or is in broadcast mode
    if (frame.addressing[1] != MAC_ADDR and frame.addressing[1] != 0xFF) {
        #ifdef DEBUG
            // put_str("Dropping frame: Destination address does not match devices\r\n");
        #endif
        return 1;
    }
    
    // TODO: Error in previous split packet frame handling
    if (frame.control[1] != 0 and error == true) {
        // Reset error flag to 0 on last split packet
        if (frame.control[0] == frame.control[1]) {
            error == false;
        }
        return 1;
    }
    
    // Error in current frame handling
    error = check_crc();
    if (error == true) {
        #ifdef DEBUG
            // put_str("Dropping frame: Error detected in frame\r\n");
        #endif
        return 1;
    }

    // Single packet (no split packets)
    if (frame.control[1] == 0) {
        #ifdef DEBUG
            // put_str("Packet: "); print(frame.net_packet, frame.length);
            allocate(received_packet, frame.length);
            memcpy(received_packet.ptr, frame.net_packet, received_packet.length);
        #endif
        // NET.receive(frame.net_packet, frame.length, frame.addressing[0]);
    // Split packet
    } else {
        // First split packet
        if (frame.control[0] == 0) {
            allocate(reconstructed_packet, frame.length);
            memcpy(reconstructed_packet.ptr, frame.net_packet, reconstructed_packet.length);
        // Nth split packet
        } else {
            // Increment reconstructed packet length
            reallocate(reconstructed_packet, reconstructed_packet.length + frame.length);
            // Append new split packet onto reconstructed packet
            memcpy(&reconstructed_packet.ptr[reconstructed_packet.length - frame.length], frame.net_packet, frame.length);
            // Last split packet
            if (frame.control[0] == frame.control[1]) {
                // NET.receive(reconstructed_packet.ptr, reconstructed_packet.length, frame.addressing[0]);
                #ifdef DEBUG
                    // put_str("Packet: "); print(reconstructed_packet);
                    allocate(received_packet, reconstructed_packet.length);
                    memcpy(received_packet.ptr, reconstructed_packet.ptr, reconstructed_packet.length);
                #endif
                // Free memory
                deallocate(reconstructed_packet);
            }
        }
    }
    deallocate(frame.net_packet, frame.length);
    
    return 0;
}

Buffer byte_stuff(Frame frame) {
    uint8_t message_length;
    uint8_t* message = NULL;
    allocate(message, message_length, 2 + 2 + 1 + frame.length + 2);
    message[0] = frame.control[0];
    message[1] = frame.control[1];
    message[2] = frame.addressing[0];
    message[3] = frame.addressing[1];
    message[4] = frame.length;
    for (uint8_t i = 0; i < frame.length; i++) {
        message[5 + i] = frame.net_packet[i];
    }
    message[message_length - 2] = frame.checksum[0];
    message[message_length - 1] = frame.checksum[1];

    for (uint8_t i = 0; i < message_length; i++) {
        // Detect FLAG or ESC
        if (message[i] == FLAG or message[i] == ESC) {
            // Increment length of message
            reallocate(message, message_length, message_length + 1);
            // Shift bytes after i right
            memcpy(&message[i + 1], &message[i], message_length - i);
            // Insert ESC at i
            message[i] = ESC;
            // XOR escaped byte
            message[i + 1] ^= 0x20;
            // Skip escaped (next) byte
            i++;
        }
    }

    Buffer stuffed_frame;
    allocate(stuffed_frame, 1 + message_length + 1);
    stuffed_frame.ptr[0] = FLAG;
    memcpy(&stuffed_frame.ptr[1], message, message_length);
    stuffed_frame.ptr[stuffed_frame.length - 1] = FLAG;

    deallocate(message, message_length);
    return stuffed_frame;
}

Frame de_byte_stuff(Buffer stuffed_frame) {
    uint8_t message_length;
    uint8_t* message = NULL;
    allocate(message, message_length, stuffed_frame.length - 2);
    memcpy(message, &stuffed_frame.ptr[1], message_length);
    
    for (uint8_t i = 0; i < message_length; i++) {
        if (message[i] == ESC) {
            // Decrement message length
            reallocate(message, message_length, message_length - 1);
            // Shift bytes left
            memcpy(&message[i], &message[i + 1], message_length - i + 1);
            // XOR unescaped byte
            message[i] ^= 0x20;
        }
    }

    Frame frame;
    frame.control[0] = message[0];
    frame.control[1] = message[1];
    frame.addressing[0] = message[2];
    frame.addressing[1] = message[3];
    allocate(frame.net_packet, frame.length, message[4]);
    memcpy(frame.net_packet, &message[5], frame.length);
    frame.checksum[0] = message[message_length - 2];
    frame.checksum[1] = message[message_length - 1];

    deallocate(message, message_length);
    return frame;
}

void DLL::calc_crc() {
    uint8_t message_length = 2 + 2 + 1 + frame.length + 2;
    uint8_t message[message_length];
    message[0] = frame.control[0];
    message[1] = frame.control[1];
    message[2] = frame.addressing[0];
    message[3] = frame.addressing[1];
    message[4] = frame.length;
    for (uint8_t i = 0; i < frame.length; i++) {
        message[5 + i] = frame.net_packet[i];
    }
    message[message_length - 2] = 0;
    message[message_length - 1] = 0;

    #define POLYNOMIAL 0x8005

    for (uint8_t bit = 0; bit < message_length*8; bit++) {
        uint8_t byte = bit/8;
        // while (message)
        // (POLYNOMIAL >> i);

        // uint8_t mask0 = 0xFF >> bit;
        // uint8_t mask1 = 0xFF << bit;
        // uint8_t& message_byte = message[byte] & mask;
    }

    frame.checksum[0] = message[message_length - 2];
    frame.checksum[1] = message[message_length - 1];

    /* #define WIDTH  (8 * sizeof(uint16_t))
    #define TOPBIT (1 << (WIDTH - 1))

    uint16_t remainder = 0;	
    // Perform modulo-2 division one byte at a time
    for (uint8_t byte = 0; byte < message_length; byte++) {
        // Bring the next byte into the remainder
        remainder ^= (message[byte] << (WIDTH - 8));

        // Perform modulo-2 division one a bit at a time
        for (uint8_t bit = 8; bit > 0; bit--) {
            // Try to divide the current message bit
            if (remainder & TOPBIT) {
                remainder = (remainder << 1) ^ POLYNOMIAL;
            } else {
                remainder = (remainder << 1);
            }
        }
    }

    // put_str("0x%04x\r\n");
 remainder);

    // The final remainder is the CRC result
    // frame.checksum[0] = (remainder & 0xFF00) >> 8;
    // frame.checksum[1] = (remainder & 0x00FF); */
}

bool DLL::check_crc() {
    return 0;
}

Frame::Frame() {
    header = FLAG;
    length = 0;
    net_packet = NULL;
    footer = FLAG;
}

Buffer::Buffer() {
    ptr = NULL;
    length = 0;
}

DLL::DLL() {
    #ifdef DEBUG
        Buffer sent_frame;
        sent_frames = &sent_frame;
        num_sent_frames = 0;
    #endif
    error = false;
}

#ifdef DEBUG
    uint8_t max(uint8_t a, uint8_t b) {
        if (a > b) {
            return a;
        } else {
            return b;
        }
    }

    void print(Frame frame) {    
        /*
        +--------+-----------+------------+--------+---------------------+------------+--------+
        | Header |  Control  | Addressing | Length |      NET Packet     |  Checksum  | Footer |
        +--------+-----------+------------+--------+---------------------+------------+--------+
        |  0x7d  | 0x7d 0x7e | 0x7d  0x7e |  0x04  | 0x7d 0x7e 0x7d 0x7e | 0x7d  0x7e |  0x7d  |
        +--------+-----------+------------+--------+---------------------+------------+--------+
        */
        uint8_t num_dashes = max(12, 1 + frame.length*5);
        uint8_t num_spaces = num_dashes - 10;
        uint8_t extra_space = num_dashes % 2;
        put_str("+--------+-----------+------------+--------+");
        if (frame.length > 0) {
            for (uint8_t i = 0; i < num_dashes; i++) {
                put_str("-");
            }
            put_str("+");
        }
        put_str("------------+--------+\r\n");
        put_str("| Header |  Control  | Addressing | Length |");
        if (frame.length > 0) {
            for (uint8_t i = 0; i < num_spaces/2 + extra_space; i++) {
                put_str(" ");
            }
            put_str("NET Packet");
            for (uint8_t i = 0; i < num_spaces/2; i++) {
                put_str(" ");
            }
            put_str("|");
        }
        put_str("  Checksum  | Footer |\r\n");
        put_str("+--------+-----------+------------+--------+");
        if (frame.length > 0) {
            for (uint8_t i = 0; i < num_dashes; i++) {
                put_str("-");
            }
            put_str("+");
        }
        put_str("------------+--------+\r\n");
        put_str("|  ");
        put_hex(frame.header);
        put_str("  | ");
        put_hex(frame.control[0]);
        put_str(" ");
        put_hex(frame.control[1]);
        put_str(" | ");
        put_hex(frame.addressing[0]);
        put_str("  ");
        put_hex(frame.addressing[1]);
        put_str(" |  ");
        put_hex(frame.length);
        put_str("  | ");
        if (frame.length > 2) {
            for (uint8_t i = 0; i < frame.length; i++) {
                put_hex(frame.net_packet[i]);
                put_str(" ");
            }
            put_str("| ");
        } else if (frame.length == 2) {
            put_hex(frame.net_packet[0]);
            put_str("  ");
            put_hex(frame.net_packet[1]);
            put_str(" | ");
        } else if (frame.length == 1) {
            put_str("   ");
            put_hex(frame.net_packet[0]);
            put_str("    | ");
        }
        put_hex(frame.checksum[0]);
        put_str("  ");
        put_hex(frame.checksum[1]);
        put_str(" |  ");
        put_hex(frame.footer);
        put_str("  |\r\n");
        put_str("+--------+-----------+------------+--------+");
        if (frame.length > 0) {
            for (uint8_t i = 0; i < num_dashes; i++) {
                put_str("-");
            }
            put_str("+");
        }
        put_str("------------+--------+\r\n");
    }

    void print(Buffer buffer) {
        /*
        +--------+---------------------+---------------------+--------+-----------------------------------------+---------------------+--------+
        | Header |       Control       |      Addressing     | Length |                NET Packet               |       Checksum      | Footer |
        +--------+---------------------+---------------------+--------+-----------------------------------------+---------------------+--------+
        |  0x7d  | 0x7e 0x5d 0x7e 0x5e | 0x7e 0x5d 0x7e 0x5e |  0x04  | 0x7e 0x5d 0x7e 0x5e 0x7e 0x5d 0x7e 0x5e | 0x7e 0x5d 0x7e 0x5e |  0x7d  |
        +--------+---------------------+---------------------+--------+-----------------------------------------+---------------------+--------+
        */
        for (uint8_t i = 0; i < buffer.length; i++) {
            put_hex(buffer.ptr[i]);
            put_str(" ");
        }
        put_str("\r\n");
    }

    void print(uint8_t* ptr, uint8_t length) {
        for (uint8_t i = 0; i < length; i++) {
            put_hex(ptr[i]);
            put_str(" ");
        }
        put_str("\r\n");
    }
#endif