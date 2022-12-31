#include "phy.hpp"
#include "dll.hpp"
#include "mem.hpp"

void PHY::send(uint8_t* frame, uint8_t frame_length) {
    #ifdef DEBUG_PHY
        put_str("\r\nSENDING BYTES\r\n");
    #endif
    for (uint8_t byte_num = 0; byte_num < frame_length; byte_num++) {
        #ifdef DEBUG_PHY_BYTES
            put_hex(frame[byte_num]); put_ch(' ');
        #endif
        // Write byte to UART
        send_byte(frame[byte_num]);
    }
    #ifdef DEBUG_PHY
        put_str("\r\n");
    #endif
}

void PHY::receive() {
    #ifdef DEBUG_PHY
        put_str("\r\nRECEIVING BYTES\r\n");
    #endif
    // Read bytes until flag byte is received
    uint8_t* frame = NULL;
    uint8_t frame_length = 0;
    allocate(frame, frame_length, 1);
    do {
        frame[0] = receive_byte();
    } while (frame[0] != FLAG);
    #ifdef DEBUG_PHY_BYTES
        put_hex(frame[0]); put_ch(' ');
    #endif
    // Store received bytes until flag byte is received
    do {
        reallocate(frame, frame_length, frame_length + 1);
        frame[frame_length - 1] = receive_byte();
        #ifdef DEBUG_PHY_BYTES
            put_hex(frame[frame_length - 1]); put_ch(' ');
        #endif
    } while (frame[frame_length - 1] != FLAG);
    #ifdef DEBUG_PHY_BYTES
        put_str("\r\n");
    #endif
    #ifdef DEBUG_PHY
        put_str("Passing frame to DLL\r\n");
    #endif
    dll->receive(frame, frame_length);
    deallocate(frame, frame_length);
}

PHY::PHY() {
    init_uart1();   // init uart
    _delay_ms(100); // delay for uart to initialize properly
}