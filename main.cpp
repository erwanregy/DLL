#include <stdlib.h>
#include "mem.hpp"
#include "config.hpp"

// #define allocate(x, ...) put_str(#x); put_str(": "); allocate(x, ##__VA_ARGS__)
// #define reallocate(x, ...) put_str(#x); put_str(": "); reallocate(x,##__VA_ARGS__)
// #define deallocate(x, ...) put_str(#x); put_str(": "); deallocate(x, ##__VA_ARGS__)

bool dll_test();
void mem_test();

int main() {
    init_uart0();   // init uart
    _delay_ms(100); // delay for uart to initialize properly
    #ifndef WINDOWS
        put_str("--------------------------------------------------------\r\n");
    #endif

    if (dll_test()) {
        put_str("Failed :(\r\n");
        return 1;
    } else {
        put_str("Success :D\r\n");
        return 0;
    }
    
    // #define NUM_TESTS 10
    // for (uint16_t i = 0; i < NUM_TESTS; i++) {
    //     if (dll_test() != 0) {
    //         put_str("Error: Test ");
    //         put_uint16(i + 1);
    //         put_str(" failed :(\r\n");
    //         return 1;
    //     } else if ((i + 1) % (NUM_TESTS/10) == 0) {
    //         put_uint16(i + 1);
    //         put_str(" tests passed\r\n");
    //     }
    // }
    // put_str("All test passed!\r\n");

    // Buffer a, b;
    // send(dll, a);
    // send(dll, a);
    // send(dll, a);
    // b = receive(dll);
    // check(a, b);
    // deallocate(a);
    // deallocate(b);
}

bool dll_test() {
    DLL dll;
    #define NUM_PACKETS 32
    #define PACKET_LENGTH 32
    uint8_t packets[NUM_PACKETS][PACKET_LENGTH];
    for (uint8_t packet_num = 0; packet_num < NUM_PACKETS; packet_num++) {
        for (uint8_t byte_num = 0; byte_num < PACKET_LENGTH; byte_num++) {
            // packets[packet_num][byte_num] = rand() % 0x100;
            packets[packet_num][byte_num] = byte_num;
            // packets[packet_num][byte_num] = rand() % 2 + FLAG;
            // packets[packet_num][byte_num] = ESC;
            // packets[packet_num][byte_num] = FLAG;
        }
        dll.send(packets[packet_num], PACKET_LENGTH, 0xFF);
        for (uint8_t frame_num = 0; frame_num < dll.num_sent_frames; frame_num++) {
            dll.receive(dll.sent_frames[frame_num], dll.sent_frame_lens[frame_num]);
            deallocate(dll.sent_frames[frame_num], dll.sent_frame_lens[frame_num]);
        }
        deallocate(dll.sent_frames, dll.sent_frame_lens, dll.num_sent_frames);;
        if (dll.received_packet_len != PACKET_LENGTH) {
            put_str("Error: Lengths do not match\r\n");
            put_str("sent_packet_len = "); put_uint8(PACKET_LENGTH);
            put_str(", received_packet_len = "); put_uint8(dll.received_packet_len); put_str("\r\n");
            exit(1);
        }
        for (uint8_t byte_num = 0; byte_num < PACKET_LENGTH; byte_num++) {
            if (dll.received_packet[byte_num] != packets[packet_num][byte_num]) {
                put_str("Error: Packets do not match\r\n");
                put_str("sent_packet["); put_uint8(byte_num); put_str("] = ");
                put_hex(packets[packet_num][byte_num]);
                put_str(", received_packet["); put_uint8(byte_num); put_str("] = ");
                put_hex(dll.received_packet[byte_num]); put_str("\r\n");
                exit(1);
            }
        }
        deallocate(dll.received_packet, dll.received_packet_len);
    }
    if (mem_leak()) {
        put_str("Error: Memory leak!\r\n");
        print_mem_use();
        exit(1);
    }
    return 0;
}

void mem_test() {
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
        allocate(ptr[i], length[i], (uint8_t) 128);
    }
    put_str("\r\n");
    for (uint8_t i = 0; i < NUM_ITERATIONS; i++) {
        for (uint8_t i = 0; i < NUM_PTRS; i++) {
            reallocate(ptr[i], length[i], (uint8_t) (rand() % MAX_LEN));
        }
        put_str("\r\n");
    }
    for (uint8_t i = 0; i < NUM_PTRS; i++) {
        deallocate(ptr[i], length[i]);
    }
}