#include "dll.hpp"
#include "mem.hpp"
#include "config.hpp"
#include <stdlib.h>

#ifdef DEBUG_MEM
    #define allocate(x, ...) put_str(#x); put_str(": "); allocate(x, ##__VA_ARGS__)
    #define reallocate(x, ...) put_str(#x); put_str(": "); reallocate(x,##__VA_ARGS__)
    #define deallocate(x, ...) put_str(#x); put_str(": "); deallocate(x, ##__VA_ARGS__)
#endif

bool dll_test(DLL&, DLL&);

int main() {
    #ifndef WINDOWS
        init_uart0();   // init uart
        _delay_ms(100); // delay for uart to initialize properly
        put_str("--------------------------------------------------------\r\n");
    #endif
    #define NUM_TESTS 10000
    #define NUM_UPDATES 100
    DLL sender, receiver;
    for (uint16_t i = 0; i < NUM_TESTS; i++) {
        bool error = dll_test(sender, receiver);
        if (error == true) {
            put_str("Test "); put_uint16(i + 1); put_str(" failed\r\n");
            return 1;
        } else if ((i + 1) % (NUM_TESTS/NUM_UPDATES) == 0) {
            if ((i + 1) == NUM_TESTS) {
                put_str("All ");
            }
            put_uint16(i + 1); put_str(" tests passed\r\n");
        }
    }
}

bool dll_test(DLL& sender, DLL& receiver) {
    uint8_t packet_length = rand() % 24 + 1;
    // uint8_t packet_length = 24;
    // uint8_t packet_length = rand() % 255 + 1;
    // uint8_t packet_length = 12;
    uint8_t packet[packet_length];
    // Initialise packet to send
    for (uint16_t byte = 0; byte < packet_length; byte++) {
        // packet[byte] = rand() % 0x100;
        // packet[byte] = rand() % 10 + 0x70;
        packet[byte] = rand() % 2 + FLAG;
        // packet[byte] = rand() % (FLAG - 1) + 1;
        // packet[byte] = byte;
    }
    #ifdef DEBUG_DLL
        print(packet, packet_length);
    #endif
    // Send packet
    sender.send(packet, packet_length, 0xFF, receiver);
    // Check received packet length matches
    if (receiver.received_packet_length != packet_length) {
        put_str("Error: lengths do not match\r\n");
        put_str("sent_packet_length = "); put_uint8(packet_length);
        put_str(", received_packet_length = "); put_uint8(receiver.received_packet_length); put_str("\r\n");
        return 1;
    }
    // Check received packet (data) matches
    for (uint8_t byte = 0; byte < packet_length; byte++) {
        if (receiver.received_packet[byte] != packet[byte]) {
            put_str("Error: Packets do not match\r\n");
            put_str("sent_packet     = "); print(packet, packet_length);
            put_str("received_packet = "); print(receiver.received_packet, receiver.received_packet_length);
            return 1;
        }
    }
    // Deallocate received packet
    deallocate(receiver.received_packet, receiver.received_packet_length);
    // Check for no memory leaks
    if (mem_leak()) {
        put_str("Error: Memory leak!\r\n");
        print_mem_use();
        return 1;
    }
    return 0;
}