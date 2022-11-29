#include <cstdint>
#include <iostream>
#include <string>
#include <bitset>

#define FLAG 0b01111110
#define ESC  0b00010000

class Frame {
public:
    uint8_t header = FLAG;
    uint8_t control[2];
    uint8_t addressing[2];
    uint8_t length;
    uint8_t* packet;
    uint8_t checksum[2];
    uint8_t footer = FLAG;

    Frame(uint8_t control[2], uint8_t addressing[2], uint8_t length, uint8_t packet[]) {
        this->control[0] = control[0];
        this->control[1] = control[1];
        this->addressing[0] = addressing[0];
        this->addressing[1] = addressing[1];
        this->length = length;
        this->packet = new uint8_t[length];
        for (uint8_t i = 0; i < length; i++) {
            this->packet[i] = packet[i];
        }
        // temp
        this->checksum[0] = 0x89;
        this->checksum[1] = 0xAB;
        byte_stuff();
    }

    void shift_bytes(uint8_t& byte) {
        for (uint8_t* byte_ptr =  + 1; byte_ptr > &byte; byte_ptr--) {
            *byte_ptr = *(byte_ptr - 1);
        }
        byte = ESC;
    }

    void byte_stuff() {
        if (control[0] == FLAG or control[0] == ESC) {
            
        }

        for (uint8_t i = 0; i < length; i++) {
            if (packet[i] == FLAG or packet[i] == ESC) {
                shift_bytes(packet[i]);
            }
        }
    }

    void print(std::string format) { // DEBUG
        if (format == "hex") {
            std::cout << std::hex << (uint)header << " | "
            << std::hex << (uint)control[0] << (uint)control[1] << " | "
            << std::hex << (uint)addressing[0] << (uint)addressing[1] << " | "
            << std::hex << (uint)length << " | ";
            for (uint8_t i = 0; i < length; i++) {
                std::cout << std::hex << (uint)packet[i];
            }
            std::cout << " | "
            << std::hex << (uint)checksum[0] << (uint)checksum[1] << " | "
            << std::hex << (uint)footer << "\n";
        } else if (format == "bin" or format == "binary") {
            std::cout << std::bitset<8>(header) << " | "
            << std::bitset<16>(control) << " | "
            << std::bitset<16>(addressing) << " | "
            << std::bitset<8>(length) << " | ";
            for (uint8_t i = 0; i < length; i++) {
                std::cout << std::bitset<8>(packet[i]);
            }
            std::cout << " | "
            << std::bitset<16>(checksum) << " | "
            << std::bitset<8>(footer) << "\n";
        } else if (format == "dec" or format == "decimal") {
            std::cout << (uint)header << " | "
            << (uint)(control[0] + control[1]) << " | "
            << (uint)(addressing[0] + addressing[1]) << " | "
            << (uint)length << " | ";
            uint packet_sum = 0;
            for (uint8_t i = 0; i < length; i++) {
                packet_sum += packet[i];
            }
            std::cout << (uint)packet_sum << " | "
            << (uint)(checksum[0] + checksum[1]) << " | "
            << (uint)footer << "\n";
        } else {
            std::cout << "Invalid format\n";
        }
    }
};

int main() {
    uint8_t control[2], addressing[2];
    uint8_t packet[5];
    control[0] = 0x01;
    control[1] = 0x23;
    addressing[0] = 0x45;
    addressing[1] = 0x67;
    packet[0] = 0xDE;
    packet[1] = 0xAD;
    packet[2] = 0xBE;
    packet[3] = 0xEF;
    packet[4] = ESC;

    Frame frame(control, addressing, 5, packet);

    frame.print("binary");
}
