#include <stdlib.h>
#include "dll.hpp"
#include "mem.hpp"
#include "config.hpp"

#ifdef DEBUG_MEM_ELABORATE
    #define allocate(x, ...) put_str(#x); put_str(": "); allocate(x, ##__VA_ARGS__)
    #define reallocate(x, ...) put_str(#x); put_str(": "); reallocate(x,##__VA_ARGS__)
    #define deallocate(x, ...) put_str(#x); put_str(": "); deallocate(x, ##__VA_ARGS__)
#endif

#define NUM_TESTS 1
#define NUM_UPDATES NUM_TESTS

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
            #ifdef DEBUG_DLL_TEST
                put_str("\r\n");
            #endif
        }
    }
}

bool dll_test(DLL& dll) {
    uint8_t packet_length = rand() % 24 + 1; // 1-24 bytes
    // uint8_t packet_length = rand() % 255 + 1; // 1-255 bytes
    uint8_t packet[packet_length];
    // Initialise packet to send
    for (uint16_t byte_num = 0; byte_num < packet_length; byte_num++) {
        packet[byte_num] = rand() % 0x100; // All possible values
        // packet[byte_num] = rand() % 0x10 + 0x70; // 0x70 to 0x7E
        // packet[byte_num] = rand() % 2 + FLAG; // FLAG and ESC
        // packet[byte_num] = byte_num; // Sequential numbering
    }
    #ifdef DEBUG_DLL_TEST
        put_str("Sending packet:  "); print(packet, packet_length);
    #endif
    // Send packet
    dll.send(packet, packet_length, 0xFF);
    // For each sent frame
    for (uint8_t frame_num = 0; frame_num < dll.num_sent_frames; frame_num++) {
        // Deallocate sent frame
        deallocate(dll.sent_frames[frame_num], dll.sent_frame_lengths[frame_num]);
    }
    #ifdef DEBUG_DLL_TEST
        put_str("\r\nReceived packet: "); print(dll.received_packet, dll.received_packet_length);
        #ifdef DEBUG_DLL
            put_str("Sent     packet: "); print(packet, packet_length);
        #endif
    #endif
    // Deallocate sent frames buffer
    deallocate(dll.sent_frames, dll.sent_frame_lengths, dll.num_sent_frames);
    // Check received packet length matches
    if (dll.received_packet_length != packet_length) {
        put_str("Error: Packet lengths do not match\r\n");
        put_str("Sent     packet length = "); put_uint8(packet_length); put_str("\r\n");
        put_str("Received packet length = "); put_uint8(dll.received_packet_length); put_str("\r\n");
        return 1;
    }
    // Check received packet (data) matches
    for (uint8_t byte_num = 0; byte_num < packet_length; byte_num++) {
        if (dll.received_packet[byte_num] != packet[byte_num]) {
            put_str("Error: Sent and received packets do not match\r\n");
            return 1;
        }
    }
    #ifdef DEBUG_DLL
        put_str("Sent and received packets match\r\n\r\n");
    #endif
    // Deallocate received packet
    deallocate(dll.received_packet, dll.received_packet_length);
    // Check for no memory leaks
    if (mem_leak()) {
        put_str("Error: Memory leak\r\n");
        print_mem_use();
        return 1;
    }
    return 0;
}