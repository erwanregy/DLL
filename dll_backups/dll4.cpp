#include "dll2.hpp"
#include <iostream>

using std::cout, std::hex;

Section::Section() {}

Section::Section(uint8_t value) {
    this->length = 1;
    this->values = new uint8_t[1];
    this->values[0] = value;
}

Section::Section(uint8_t length, uint8_t* values) {
    this->length = length;
    this->values = new uint8_t[length];
    for (uint8_t i = 0; i < length; i++) {
        this->values[i] = values[i];
    }
}

Section::~Section() {
    delete values;
}

void Section::byte_stuff() {
    for (uint8_t i = 0; i < length; i++) {
        if (values[i] == FLAG || values[i] == ESC) {
            uint8_t* temp = new uint8_t[length];
            temp = values;
            length++;
            values = new uint8_t[length];
            for (uint8_t j = 0; j < i; j++) {
                values[j] = temp[j];
            }
            values[i] = ESC;
            for (uint8_t j = i + 1; j < length; j++) {
                values[j] = temp[j-1];
            }
            i++; // skip a byte
            delete temp;
        }
    }
}

void Section::de_byte_stuff() {
    for (uint8_t i = 0; i < length - 1; i++) {
        if (values[i] == ESC && (values[i+1] == FLAG || values[i+1] == ESC)) {
            uint8_t* temp = new uint8_t[length];
            temp = values;
            length--;
            values = new uint8_t[length];
            for (uint8_t j = 0; j < i; j++) {
                values[j] = temp[j];
            }
            for (uint8_t j = i; j < length; j++) {
                values[j] = temp[j+1];
            }
        }
    }
}

void Section::print() {
    for (uint8_t i = 0; i < length; i++) {
        cout << hex << (uint32_t)values[i] << " ";
    }
    cout << "| ";
}

Frame::Frame(uint8_t* control, uint8_t* addressing, uint8_t length, uint8_t* net_packet)
: control(2, control), addressing(2, addressing), length(length), net_packet(length, net_packet), checksum(2, calc_crc()) {
    // byte_stuff();
}

uint8_t* Frame::calc_crc() {
    uint8_t* temp = new uint8_t[2];
    temp[0] = FLAG;
    temp[1] = ESC;
    return temp;
}

void Frame::byte_stuff() {
    control.byte_stuff();
    addressing.byte_stuff();
    net_packet.byte_stuff();
    length.values[0] = net_packet.length;
    length.byte_stuff();
    checksum.byte_stuff();
}

void Frame::de_byte_stuff() {
    control.de_byte_stuff();
    addressing.de_byte_stuff();
    net_packet.de_byte_stuff();
    length.de_byte_stuff();
    length.values[0] = net_packet.length;
    checksum.de_byte_stuff();
}

void Frame::print() {
    cout << "| " << hex << (uint32_t)header << " | ";
    control.print();
    addressing.print();
    length.print();
    net_packet.print();
    checksum.print();
    cout << hex << (uint32_t)footer << " |\n";
}

int main() {
    uint8_t* control = new uint8_t[2];
    uint8_t* addressing = new uint8_t[2];
    uint8_t length = 1;
    uint8_t* net_packet = new uint8_t[length];

    control[0] = FLAG;
    control[1] = ESC;
    addressing[0] = FLAG;
    addressing[1] = ESC;
    net_packet[0] = FLAG;
    net_packet[1] = ESC;
    net_packet[2] = FLAG;
    net_packet[3] = ESC;

    cout << "Create frame:\n";
    Frame frame(control, addressing, length, net_packet);
    frame.print();
    cout << "\n";

    cout << "Byte stuff:\n";
    frame.byte_stuff();
    frame.print();
    cout << "\n";

    cout << "De byte stuff:\n";
    frame.de_byte_stuff();
    frame.print();
}