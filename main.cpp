#include <stdlib.h>
#include "dll.hpp"
#include "mem.hpp"
#include "config.hpp"

#ifdef DEBUG_MEM
    #define allocate(x, ...) put_str(#x); put_str(": "); allocate(x, ##__VA_ARGS__)
    #define reallocate(x, ...) put_str(#x); put_str(": "); reallocate(x,##__VA_ARGS__)
    #define deallocate(x, ...) put_str(#x); put_str(": "); deallocate(x, ##__VA_ARGS__)
#endif

bool dll_test();
bool mem_test();

int main() {
    init_uart0();   // init uart
    _delay_ms(100); // delay for uart to initialize properly
    put_str("--------------------------------------------------------\r\n");
    #define NUM_TESTS 1
    #define NUM_UPDATES 1
    for (uint16_t i = 0; i < NUM_TESTS; i++) {
        if (dll_test()) {
            put_str("Test "); put_uint16(i + 1); put_str(" failed\r\n");
            return 1;
        } else if ((i + 1) % (NUM_TESTS/NUM_UPDATES) == 0) {
            if (i == NUM_TESTS - 1) {
                put_str("All ");
            }
            put_uint16(i + 1); put_str(" tests passed\r\n");
        }
    }
    return 0;
}

bool dll_test() {
    DLL dll;
    #define PACKET_LENGTH 4
    uint8_t packet[PACKET_LENGTH];
    // Initialise packet to send
    for (uint8_t byte_num = 0; byte_num < PACKET_LENGTH; byte_num++) {
        // packet[byte_num] = rand() % 0x100;
        // packet[byte_num] = byte_num;
        // packet[byte_num] = rand() % 2 + FLAG;
        packet[byte_num] = FLAG;
        // packet[++byte_num] = ESC;
    }
    put_str("TX packet: "); print(packet, PACKET_LENGTH);
    // Send packet
    dll.send(packet, PACKET_LENGTH, 0xFF);
    // For each sent frame
    for (uint8_t frame_num = 0; frame_num < dll.num_sent_frames; frame_num++) {
        // Receive frame
        dll.receive(dll.sent_frames[frame_num], dll.sent_frame_lens[frame_num]);
        // Deallocate sent frame
        deallocate(dll.sent_frames[frame_num], dll.sent_frame_lens[frame_num]);
    }
    // Deallocate sent frames buffer
    deallocate(dll.sent_frames, dll.sent_frame_lens, dll.num_sent_frames);
    put_str("RX packet: "); print(dll.received_packet, dll.received_packet_len);
    // Check received packet length matches
    if (dll.received_packet_len != PACKET_LENGTH) {
        put_str("Error: Lengths do not match\r\n");
        put_str("sent_packet_len = "); put_uint8(PACKET_LENGTH);
        put_str(", received_packet_len = "); put_uint8(dll.received_packet_len); put_str("\r\n");
        return 1;
    }
    // Check received packet (data) matches
    for (uint8_t byte_num = 0; byte_num < PACKET_LENGTH; byte_num++) {
        if (dll.received_packet[byte_num] != packet[byte_num]) {
            put_str("Error: Packets do not match\r\n");
            put_str("sent_packet["); put_uint8(byte_num); put_str("] = "); put_hex(packet[byte_num]);
            put_str(", received_packet["); put_uint8(byte_num); put_str("] = "); put_hex(dll.received_packet[byte_num]); put_str("\r\n");
            for (byte_num++; byte_num < PACKET_LENGTH; byte_num++) {
                if (dll.received_packet[byte_num] != packet[byte_num]) {
                    put_str("sent_packet["); put_uint8(byte_num); put_str("] = "); put_hex(packet[byte_num]);
                    put_str(", received_packet["); put_uint8(byte_num); put_str("] = "); put_hex(dll.received_packet[byte_num]); put_str("\r\n");
                }
            }
            return 1;
        }
    }
    // Deallocate received packet
    deallocate(dll.received_packet, dll.received_packet_len);
    // Check for no memory leaks
    if (mem_leak()) {
        put_str("Error: Memory leak!\r\n");
        print_mem_use();
        return 1;
    }
    return 0;
}

bool mem_test() {
    #define NUM_PTRS 24
    #define MAX_LEN 128
    #define NUM_ITERATIONS 3
    uint8_t* ptr[NUM_PTRS];
    uint8_t length[NUM_PTRS];
    for (uint8_t i = 0; i < NUM_PTRS; i++) {
        ptr[i] = NULL;
        length[i] = 0;
    }
    for (uint8_t i = 0; i < NUM_PTRS; i++) {
        allocate(ptr[i], length[i], 128);
    }
    put_str("\r\n");
    for (uint8_t i = 0; i < NUM_ITERATIONS; i++) {
        for (uint8_t i = 0; i < NUM_PTRS; i++) {
            reallocate(ptr[i], length[i], rand() % MAX_LEN);
        }
        put_str("\r\n");
    }
    for (uint8_t i = 0; i < NUM_PTRS; i++) {
        deallocate(ptr[i], length[i]);
    }
    if (mem_leak()) {
        put_str("Error: Memory leak!\r\n");
        print_mem_use();
        return 1;
    }
    return 0;
}