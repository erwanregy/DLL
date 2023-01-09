#include "dll.hpp"
#include "rfm12.h"
#include "config.hpp"
#include <stdlib.h>
#include <util/delay.h>

void DLL::MAC::send(uint8_t* frame, uint8_t frame_length) {
    // Queue frame in PHY buffer
    rfm12_tx(frame_length, 0, frame);
}

void DLL::MAC::tick() {
    // If PHY channel is free  
    if (ctrl.rfm12_state == STATUS_FREE) {
        #ifdef DEBUG_MAC
            put_str("MAC: PHY is free\r\n");
        #endif

        #ifdef PRINT_MAC_STEPS
            put_str("Checking back off counter...\r\n")
        #endif
        // If back off counter has expired
        if (back_off_counter == 0) {
            #ifdef PRINT_MAC_STEPS
                put_str("Back off counter expired\r\n");
                put_str("Deciding whether to transmit with 0.5 probability...\r\n");
            #endif

            bool transmit = rand() % 2;
            // Transmit with 0.5 probability
            if (transmit == true) {
                #ifdef DEBUG_MAC
                    put_str("Transmitting\r\n");
                #endif
                rfm12_start_tx();

                // If a collision is detected
                if (rfm12_detect_collision() == true) {
                    #ifdef DEBUG_MAC
                        put_str("Collision detected\r\n");
                    #endif

                    // Reset the back off counter to a random value
                    back_off_counter = rand() % 0xFF;
                    #ifdef PRINT_MAC_STEPS
                        put_str("Reset back off counter to random value "); put_uint8(back_off_counter); put_str("\r\n")
                    #endif
                }
            }
            #ifdef PRINT_MAC_STEPS
                else {
                    #ifdef PRINT_MAC_STEPS
                        put_str("Not transmitting\r\n");
                    #endif
                }
            #endif
        } else {
            #ifdef PRINT_MAC_STEPS
                put_str("Back off counter not expired, decrementing\r\n");
            #endif
            // Decrement back off counter
            back_off_counter--;
        }
    }
    // Else wait till next timeslot
    #ifdef PRINT_MAC_STEPS
        put_str("MAC: PHY is not free, waiting till next timeslot\r\n");
    #endif
}

void DLL::MAC::receive() {
    // If the receiver has received a message
    if (rfm12_rx_status() == STATUS_OCCUPIED) {
        #ifdef DEBUG_MAC
            put_str("MAC: Receiving frame\r\n");
        #endif

        // Get the frame pointer and length
        uint8_t* frame = rfm12_rx_buffer();
        uint8_t frame_length = rfm12_rx_len();

        // Pass frame to LLC
        #ifdef DEBUG_MAC_STEPS
            put_str("Passing frame to LLC\r\n");
        #endif
        dll.receive(frame, frame_length);

        // Clear buffer
        rfm12_rx_clear();
    }
}

DLL::MAC::MAC(DLL& dll) : dll(dll) {
    back_off_counter = 0;
    rfm12_init();   // Initialise the RFM12
    _delay_ms(100); // Delay for the RFM12 to initialize properly
}