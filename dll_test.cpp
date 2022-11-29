#include "dll_test.hpp"
#include "mem.hpp"
#include <stdlib.h>

// #define allocate(x, ...) put_str(#x); put_str(": "); allocate(x, ##__VA_ARGS__)
// #define reallocate(x, ...) put_str(#x); put_str(": "); reallocate(x,##__VA_ARGS__)
// #define deallocate(x, ...) put_str(#x); put_str(": "); deallocate(x, ##__VA_ARGS__)

bool dll_test() {
    #define NUM_PACKETS 1
    #define PACKET_LENGTH 12
    DLL dll;
    Buffer sent_packet, received_packet;
    send_packet(dll, sent_packet);
    received_packet = receive_packet(dll);
    // Check all packets match
    check_packets(sent_packet, received_packet);
    deallocate(sent_packet);
    deallocate(received_packet);
    if (mem_use != 0) {
        put_str("Error: Memory leak!\r\n");
        print_mem_use();
        exit(1);
    }
    return 0;
}

void send_packet(DLL& dll, Buffer& packet) {
    // put_str("TX:\r\n");
    allocate(packet, PACKET_LENGTH);
    for (uint8_t i = 0; i < packet.length; i++) {
        // packet.ptr[i] = rand() % 0x100;
        packet.ptr[i] = i;
        // packet.ptr[i] = rand() % 3 + FLAG;
    }
    // put_str("Packet: "); print(packet.ptr, packet.length);
    dll.send(packet.ptr, packet.length, MAC_ADDR);
}

Buffer receive_packet(DLL& dll) {
    // put_str("RX:\r\n");
    for (uint8_t i = 0; i < dll.num_sent_frames; i++) {
        dll.receive(dll.sent_frames[i].ptr, dll.sent_frames[i].length);
        deallocate(dll.sent_frames[i]);
    }
    deallocate(dll.sent_frames, dll.num_sent_frames);
    return dll.received_packet;
}

bool check_packets(Buffer sent, Buffer received) {
    if (sent.length != received.length) {
        put_str("Error: Lengths do not match\r\n");
        put_str("sent_packet.length = "); put_uint8(sent.length);
        put_str(", received_packet.length = "); put_uint8(received.length); put_str("\r\n");
        exit(1);
    }
    for (uint8_t i = 0; i < sent.length; i++) {
        if (sent.ptr[i] != received.ptr[i]) {
            put_str("Error: Packets do not match\r\n");
            put_str("sent_packet.ptr["); put_uint8(i); put_str("] = "); put_hex(sent.ptr[i]);
            put_str(", received_packet.ptr["); put_uint8(i); put_str("] = "); put_hex(received.ptr[i]); put_str("\r\n");
            exit(1);
        }
    }
    return 0;
}
