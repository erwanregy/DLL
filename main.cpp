#include <stdlib.h>
#include "dll.hpp"
#include "mem.hpp"
#include "config.hpp"

#ifdef DEBUG_MEM
    #define allocate(x, ...) put_str(#x); put_str(": "); allocate(x, ##__VA_ARGS__)
    #define reallocate(x, ...) put_str(#x); put_str(": "); reallocate(x,##__VA_ARGS__)
    #define deallocate(x, ...) put_str(#x); put_str(": "); deallocate(x, ##__VA_ARGS__)
#endif

#define NUM_TESTS 100
#define NUM_UPDATES 10

bool dll_test(DLL&);

int main() {
    #ifndef WINDOWS
        init_uart0();   // init uart
        _delay_ms(100); // delay for uart to initialize properly
        put_str("--------------------------------------------------------\r\n");
    #endif
    // Test DLL
    DLL dll;
    for (uint16_t i = 0; i < NUM_TESTS; i++) {
        bool error = dll_test(dll);
        if (error == true) {
            put_str("Test "); put_uint16(i + 1); put_str(" failed\r\n");
            return 1;
        } else if ((i + 1) % (NUM_TESTS/NUM_UPDATES) == 0) {
            if (i == NUM_TESTS - 1) {
                put_str("All ");
            }
            put_uint16(i + 1); put_str(" tests passed\r\n");
        }
    }
}

bool dll_test(DLL& dll) {
    uint8_t packet_length = rand() % 24 + 1;
    // uint8_t packet_length = 24;
    // uint8_t packet_length = rand() % 255 + 1;
    // uint8_t packet_length = 12;
    uint8_t packet[packet_length];
    // Initialise packet to send
    for (uint16_t byte_num = 0; byte_num < packet_length; byte_num++) {
        packet[byte_num] = rand() % 0x100;
        // packet[byte_num] = rand() % 10 + 0x70;
        // packet[byte_num] = rand() % 2 + FLAG;
        // packet[byte_num] = FLAG;
        // packet[byte_num] = rand() % (FLAG - 1) + 1;
        // packet[byte_num] = byte_num;
    }
    #ifdef DEBUG_DLL
        print(packet, packet_length);
    #endif
    // Send packet
    dll.send(packet, packet_length, 0xFF);
    // For each sent frame
    for (uint8_t frame_num = 0; frame_num < dll.num_sent_frames; frame_num++) {
        // Deallocate sent frame
        deallocate(dll.sent_frames[frame_num], dll.sent_frame_lengths[frame_num]);
    }
    // Deallocate sent frames buffer
    deallocate(dll.sent_frames, dll.sent_frame_lengths, dll.num_sent_frames);
    // Check received packet length matches
    if (dll.received_packet_length != packet_length) {
        put_str("Error: lengths do not match\r\n");
        put_str("sent_packet_length = "); put_uint8(packet_length);
        put_str(", received_packet_length = "); put_uint8(dll.received_packet_length); put_str("\r\n");
        return 1;
    }
    // Check received packet (data) matches
    for (uint8_t byte_num = 0; byte_num < packet_length; byte_num++) {
        if (dll.received_packet[byte_num] != packet[byte_num]) {
            put_str("Error: Packets do not match\r\n");
            put_str("sent_packet     = "); print(packet, packet_length);
            put_str("received_packet = "); print(dll.received_packet, dll.received_packet_length);
            return 1;
        }
    }
    // Deallocate received packet
    deallocate(dll.received_packet, dll.received_packet_length);
    // Check for no memory leaks
    if (mem_leak()) {
        put_str("Error: Memory leak!\r\n");
        print_mem_use();
        return 1;
    }
    return 0;
}