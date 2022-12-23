#include "dll.hpp"
#include "mem.hpp"
#include <string.h>

#ifdef DEBUG_MEM_ELABORATE
    #define allocate(x, ...) put_str(#x); put_str(": "); allocate(x, ##__VA_ARGS__)
    #define reallocate(x, ...) put_str(#x); put_str(": "); reallocate(x,##__VA_ARGS__)
    #define deallocate(x, ...) put_str(#x); put_str(": "); deallocate(x, ##__VA_ARGS__)
#endif

void DLL::send(uint8_t* packet, uint8_t packet_length, uint8_t destination_address) {
    bool extra_frame = packet_length % MAX_PACKET_LENGTH;
    uint8_t last_frame_num = packet_length/MAX_PACKET_LENGTH + extra_frame - 1;
    for (uint8_t frame_num = 0; frame_num <= last_frame_num; frame_num++) {
        #ifdef DEBUG_DLL
            put_str("\r\nSENDING FRAME\r\n");
        #endif
        uint8_t frame_packet_length;
        if (frame_num == last_frame_num) {
            frame_packet_length = packet_length - last_frame_num*MAX_PACKET_LENGTH;
        } else {
            frame_packet_length = MAX_PACKET_LENGTH;
        }
        frame.control[0] = frame_num;
        frame.control[1] = last_frame_num;
        #ifdef DEBUG_DLL_STEPS
            if (last_frame_num > 0) {
                put_str("Frame sending as ");
                if (frame_num < last_frame_num) {
                    put_str("part of");
                } else {
                    put_str("final");
                }
                put_str(" split packet "); put_uint8(frame.control[0]+1); put_ch('/'); put_uint8(frame.control[1]+1); put_str("\r\n"); 
            }
        #endif
        frame.addressing[0] = MAC_ADDRESS;
        #ifdef DEBUG_DLL_STEPS
            put_str("Device address: "); put_hex(frame.addressing[0]); put_str("\r\n");
        #endif
        frame.addressing[1] = destination_address;
        #ifdef DEBUG_DLL_STEPS
            put_str("Destination address: "); put_hex(frame.addressing[1]); put_str("\r\n");
        #endif
        allocate(frame.net_packet, frame.length, frame_packet_length);
        for (uint8_t i = 0; i < frame_packet_length; i++) {
            frame.net_packet[i] = packet[frame_num*MAX_PACKET_LENGTH + i]; 
        }
        #ifdef DEBUG_DLL_STEPS
            put_str("NET packet length: "); put_uint8(frame.length); put_str("\r\n");
            put_str("NET packet: "); print(frame.net_packet, frame.length);
            put_str("Calculating CRC...\r\n");
        #endif
        uint16_t crc = calculate_crc();
        frame.checksum[0] = (crc & 0xFF00) >> 8;
        frame.checksum[1] = (crc & 0x00FF);
        #ifdef DEBUG_DLL_STEPS
            put_str("CRC: "); put_hex(frame.checksum[0]); put_ch(' '); put_hex(frame.checksum[1]); put_str("\r\n");
        #endif
        #ifdef DEBUG_DLL_FRAMES
            put_str("Constructed frame:\r\n");
            print(frame);
        #endif
        #ifdef DEBUG_DLL_STEPS
            put_str("Stuffing bytes...\r\n");
        #endif
        byte_stuff(); // allocates memory
        #ifdef DEBUG_DLL_FRAMES
            put_str("Stuffed frame:\r\n"); print(stuffed_frame, stuffed_frame_length);
        #endif
        deallocate(frame.net_packet, frame.length);
        #ifndef DLL_TEST
            #ifdef DEBUG_DLL
                put_str("Passing frame to PHY\r\n");
            #endif
            phy->send(stuffed_frame, stuffed_frame_length);
        #else
            #ifdef DEBUG_DLL
                put_str("Passing frame to virtual DLL\r\n");
            #endif
            receive(stuffed_frame, stuffed_frame_length);
        #endif
        // Deallocate the stuffed frame
        deallocate(stuffed_frame, stuffed_frame_length);
    }
}

void DLL::receive(uint8_t* received_frame, uint8_t received_frame_length) {
    #ifdef DEBUG_DLL
        put_str("\r\nRECEIVING FRAME\r\n");
    #endif
    #ifndef DLL_TEST
        allocate(stuffed_frame, stuffed_frame_length, received_frame_length);
    #endif
    memcpy(stuffed_frame, received_frame, stuffed_frame_length);

    #ifdef DEBUG_DLL_FRAMES
        put_str("Stuffed frame:\r\n");
        print(stuffed_frame, stuffed_frame_length);
    #endif
    #ifdef DEBUG_DLL_STEPS
        put_str("Destuffing bytes...\r\n");
    #endif
    de_byte_stuff(); // allocates memory
    #ifdef DEBUG_DLL_FRAMES
        put_str("Received frame:\r\n");
        print(frame);
    #endif
    deallocate(stuffed_frame, stuffed_frame_length);

    #ifdef DEBUG_DLL_STEPS
        put_str("Checking destination address...\r\n");
        put_str("Destination address: "); put_hex(frame.addressing[1]); put_str(", ");
        put_str("Device address: "); put_hex(MAC_ADDRESS); put_str("\r\n");
    #endif
    // Check that destination MAC address in frame matches local MAC address or is in broadcast mode
    if (frame.addressing[1] != MAC_ADDRESS and frame.addressing[1] != 0xFF) {
        #ifdef DEBUG_DLL_STEPS
            put_str("Destination address check failed\r\n"); 
        #endif
        #ifdef DEBUG_DLL
            put_str("Dropping frame: Destination address does not match devices\r\n");
        #endif
        return;
    }
    #ifdef DEBUG_DLL_STEPS
        put_str("Destination address check passed\r\n"); 
    #endif
    // Error in split packet handling
    if (split_packet_error == true) {
        // Reset error flag to 0 on last split packet
        if (frame.control[0] == frame.control[1]) {
            split_packet_error = false;
        }
        #ifdef DEBUG_DLL
            put_str("Dropping frame: Error detected in previous split packet frame\r\n");
        #endif
        // Drop all split packet frames
        return;
    }
    
    #ifdef DEBUG_DLL_STEPS
        put_str("Checking CRC...\r\n"); 
    #endif
    // Error in current frame handling
    bool frame_error = check_crc();
    if (frame_error == true) {
        #ifdef DEBUG_DLL_STEPS
            put_str("CRC check failed\r\n"); 
        #endif
        if (frame.control[1] != 0) {
            #ifdef DEBUG_DLL
                put_str("Dropping frame: Error detected in split packet frame\r\n");
            #endif
            split_packet_error = true;
        } else {
            #ifdef DEBUG_DLL
                put_str("Dropping frame: Error detected in frame\r\n");
            #endif
        }
        return;
    }
    #ifdef DEBUG_DLL_STEPS
        put_str("CRC check passed\r\n"); 
    #endif


    // Single packet (no split packets)
    if (frame.control[1] == 0) {
        #ifdef DEBUG_DLL
            put_str("Received packet: "); print(frame.net_packet, frame.length);
        #endif
        #ifndef DLL_TEST
            #ifdef DEBUG_DLL
                put_str("Passing packet to NET\r\n");
            #endif
            net->receive(frame.net_packet, frame.length, frame.addressing[0]);
        #else
            allocate(received_packet, received_packet_length, frame.length);
            memcpy(received_packet, frame.net_packet, received_packet_length);
        #endif
        #ifdef DEBUG_DLL
            put_str("\r\n");
        #endif
    // Split packet
    } else {
        #ifdef DEBUG_DLL_STEPS
            put_str("Frame detected as ");
            if (frame.control[0] < frame.control[1]) {
                put_str("part of");
            } else {
                put_str("final");
            }
            put_str(" split packet "); put_uint8(frame.control[0]+1); put_ch('/'); put_uint8(frame.control[1]+1); put_str("\r\n"); 
        #endif
        // First split packet
        if (frame.control[0] == 0) {
            allocate(reconstructed_packet, reconstructed_packet_length, frame.length);
            memcpy(reconstructed_packet, frame.net_packet, reconstructed_packet_length);
            #ifdef DEBUG_DLL_STEPS
                put_str("Split packet: "); print(frame.net_packet, frame.length);
                put_str("Storing split packet...\r\n"); 
            #endif
            #ifdef DEBUG_DLL
                put_str("Partially reconstructed packet: "); print(reconstructed_packet, reconstructed_packet_length);
            #endif
        // Nth split packet
        } else {
            // Increment reconstructed packet length
            reallocate(reconstructed_packet, reconstructed_packet_length, reconstructed_packet_length + frame.length);
            // Append new split packet onto reconstructed packet
            memcpy(&reconstructed_packet[reconstructed_packet_length - frame.length], frame.net_packet, frame.length);
            // Last split packet
            #ifdef DEBUG_DLL_STEPS
                put_str("Split packet: "); print(frame.net_packet, frame.length);
                put_str("Storing split packet...\r\n");
            #endif
            #ifdef DEBUG_DLL
                if (frame.control[0] < frame.control[1]) {
                    put_str("Partially");
                } else {
                    put_str("Fully");
                }
                put_str(" reconstructed packet: "); print(reconstructed_packet, reconstructed_packet_length);
            #endif
            if (frame.control[0] == frame.control[1]) {
                #ifndef DLL_TEST
                    #ifdef DEBUG_DLL
                        put_str("Passing packet to NET...\r\n");
                    #endif
                    net->receive(reconstructed_packet, reconstructed_packet_length, frame.addressing[0]);
                #else
                    allocate(received_packet, received_packet_length, reconstructed_packet_length);
                    memcpy(received_packet, reconstructed_packet, reconstructed_packet_length);
                #endif
                #ifdef DEBUG_DLL
                    put_str("\r\n");
                #endif
                // Free memory
                deallocate(reconstructed_packet, reconstructed_packet_length);
            }
        }
    }
    deallocate(frame.net_packet, frame.length);
}

void DLL::byte_stuff() {
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
    // print(message, message_length);

    for (uint8_t i = 0; i < message_length; i++) {
        // Detect FLAG or ESC
        if (message[i] == FLAG or message[i] == ESC) {
            #ifdef DEBUG_DLL_STEPS
                print(message, message_length);
                put_str("Escaping ");
                if (message[i] == FLAG) {
                    put_str("flag");
                } else {
                    put_str("escape");
                }
                put_str(" byte detected at byte "); put_uint8(i+1); put_str("...\r\n");
            #endif
            // Increment length of message
            reallocate(message, message_length, message_length + 1);
            // print(message, message_length);
            // Shift bytes after i right
            uint8_t temp[message_length - i];
            memcpy(temp, &message[i], message_length - i);
            memcpy(&message[i + 1], temp, message_length - i);
            // Insert ESC at i
            message[i] = ESC;
            // XOR escaped byte
            // message[i + 1] ^= 0x20;
            // Skip escaped (next) byte
            i++;
            // print(message, message_length);
            #ifdef DEBUG_DLL_STEPS
                print(message, message_length);
            #endif
        }
    }

    allocate(stuffed_frame, stuffed_frame_length, 1 + message_length + 1);
    stuffed_frame[0] = FLAG;
    memcpy(&stuffed_frame[1], message, message_length);
    stuffed_frame[stuffed_frame_length - 1] = FLAG;

    deallocate(message, message_length);
}

void DLL::de_byte_stuff() {
    uint8_t message_length;
    uint8_t* message = NULL;
    allocate(message, message_length, stuffed_frame_length - 2);
    memcpy(message, &stuffed_frame[1], message_length);
    
    for (uint8_t i = 0; i < message_length; i++) {
        if (message[i] == ESC) {
            #ifdef DEBUG_DLL_STEPS
                print(message, message_length);
                put_str("Removing escape byte detected at byte "); put_uint8(i+1); put_str("...\r\n"); 
            #endif
            // Shift bytes after i left
            memcpy(&message[i], &message[i + 1], message_length - i);
            // XOR de-escaped byte
            // message[i] ^= 0x20;
            // Decrement message length
            reallocate(message, message_length, message_length - 1);
            #ifdef DEBUG_DLL_STEPS
                print(message, message_length);
            #endif
        }
    }

    frame.control[0] = message[0];
    frame.control[1] = message[1];
    frame.addressing[0] = message[2];
    frame.addressing[1] = message[3];
    allocate(frame.net_packet, frame.length, message[4]);
    memcpy(frame.net_packet, &message[5], frame.length);
    frame.checksum[0] = message[message_length - 2];
    frame.checksum[1] = message[message_length - 1];

    deallocate(message, message_length);
}

uint16_t DLL::calculate_crc() {
    // Copy frame contents into a byte array
    uint8_t message_length = 2 + 2 + 1 + frame.length;
    uint8_t message[message_length];
    message[0] = frame.control[0];
    message[1] = frame.control[1];
    message[2] = frame.addressing[0];
    message[3] = frame.addressing[1];
    message[4] = frame.length;
    for (uint8_t i = 0; i < frame.length; i++) {
        message[5 + i] = frame.net_packet[i];
    }
    #ifdef DEBUG_DLL_STEPS
        put_str("Message: "); print(message, message_length);
        put_str("Polynomial: "); put_hex(POLYNOMIAL); put_str("\r\n");
    #endif
    // Initialize the value of the CRC to 0
    uint16_t crc = 0;
    // Perform modulo-2 division, a byte at a time.
    for (int byte = 0; byte < message_length; byte++) {
        // Bring the next byte into the crc.
        crc ^= message[byte] << 8;
        // Perform modulo-2 division, a bit at a time.
        for (uint8_t bit = 8; bit > 0; bit--) {
            // Try to divide the current data bit.
            if (crc & (1 << 15)) {
                crc = (crc << 1) ^ POLYNOMIAL;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

bool DLL::check_crc() {
    uint16_t crc = calculate_crc();
    uint8_t expected_crc[2];
    expected_crc[0] = (crc & 0xFF00) >> 8;
    expected_crc[1] = (crc & 0x00FF);
    #ifdef DEBUG_DLL_STEPS
        put_str("Expected CRC: "); put_hex(expected_crc[0]); put_ch(' '); put_hex(expected_crc[1]); put_str("\r\n");
        put_str("Received CRC: "); put_hex(frame.checksum[0]); put_ch(' '); put_hex(frame.checksum[1]); put_str("\r\n");
    #endif
    if (frame.checksum[0] != expected_crc[0] or frame.checksum[1] != expected_crc[1]) {
        return 1;
    } else {
        return 0;
    }
}

Frame::Frame() {
    header = FLAG;
    length = 0;
    net_packet = NULL;
    footer = FLAG;
}

DLL::DLL() {
    stuffed_frame = NULL;
    stuffed_frame_length = 0;
    reconstructed_packet = NULL;
    reconstructed_packet_length = 0;
    #ifdef DLL_TEST
        received_packet = NULL;
        received_packet_length = 0;
    #endif
    split_packet_error = false;
}

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
            put_ch('-');
        }
        put_ch('+');
    }
    put_str("------------+--------+\r\n");
    put_str("| Header |  Control  | Addressing | Length |");
    if (frame.length > 0) {
        for (uint8_t i = 0; i < num_spaces/2 + extra_space; i++) {
            put_ch(' ');
        }
        put_str("NET Packet");
        for (uint8_t i = 0; i < num_spaces/2; i++) {
            put_ch(' ');
        }
        put_ch('|');
    }
    put_str("  Checksum  | Footer |\r\n");
    put_str("+--------+-----------+------------+--------+");
    if (frame.length > 0) {
        for (uint8_t i = 0; i < num_dashes; i++) {
            put_ch('-');
        }
        put_ch('+');
    }
    put_str("------------+--------+\r\n");
    put_str("|  ");
    put_hex(frame.header);
    put_str("  | ");
    put_hex(frame.control[0]);
    put_ch(' ');
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
            put_ch(' ');
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
            put_ch('-');
        }
        put_ch('+');
    }
    put_str("------------+--------+\r\n");
}

void print(uint8_t* ptr, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        put_hex(ptr[i]);
        put_ch(' ');
    }
    put_str("\r\n");
}