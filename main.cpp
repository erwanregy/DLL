#include "dll_test.hpp"
#include "dll.hpp"
#include "phy.hpp"
#include <stdlib.h>

int main() {
    #ifndef WINDOWS
        init_uart0();   // init uart
        put_str("\r\n--------------------------------------------------------\r\n");
    #endif
    #ifdef VIRTUAL_DLL
        dll_test();
    #else
        DLL dll;
        PHY phy;
        dll.phy = &phy;
        phy.dll = &dll;
        while (true) {
            uint8_t packet_length = rand() % 8 + 1;
            uint8_t packet[packet_length];
            for (uint8_t byte_num = 0; byte_num < packet_length; byte_num++) {
                packet[byte_num] = byte_num;
            }
            put_str("\r\nSENDING PACKET\r\n");
            print(packet, packet_length);
            dll.send(packet, packet_length, 0xFF);
            phy.receive();
            _delay_ms(2000);
        }
    #endif
}