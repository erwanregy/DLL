#include <cstdint>
#include <iostream>
#include <vector>

using std::cout;
using std::hex;
using std::vector;

#define byte uint8_t
#define FLAG 0b01111110
#define ESC  0b00010000

class Frame {
public:
    byte header = FLAG;
    vector<byte> control;
    vector<byte> addressing;
    vector<byte> length;
    vector<byte> packet;
    vector<byte> checksum;
    byte footer = FLAG;

    Frame(vector<byte> control, vector<byte> addressing, vector<byte> length, vector<byte> packet)
    : control(control), addressing(addressing), length(length), packet(packet) {
        checksum = calc_crc();
        byte_stuff();
    }

    vector<byte> calc_crc() {
        return {0x9a, 0xbc};
    }

    void byte_stuff(vector<byte>& vec) {
        // for (auto it = vec.begin(); it < vec.end(); it++) {
        //     if (*it == FLAG || *it == ESC) {
        //         vec.insert(it, ESC);
        //     }
        // }

        byte i = 0;
        for (byte v: vec) {
            if (v == FLAG || v == ESC) {
                vec.insert(vec.begin() + i, ESC);
            }
            i++;
        }
    }

    void byte_stuff() {
        byte_stuff(control);
        byte_stuff(addressing);
        byte_stuff(length);
        byte_stuff(packet);
        byte_stuff(checksum);
    }

    void print() { // DEBUG
        cout << hex << (uint)header << " | ";
        for (byte con: control) {
            cout << hex << (uint)con << " ";
        }
        cout << "| ";
        for (byte addr: addressing) {
            cout << hex << (uint)addr << " ";
        }
        cout << "| ";
        for (byte len: length) {
            cout << hex << (uint)len << " ";
        }
        cout << "| ";
        for (byte pack: packet) {
            cout << hex << (uint)pack << " ";
        }
        cout << "| ";
        for (byte check: checksum) {
            cout << hex << (uint)check << " ";
        }
        cout << "| ";
        cout << hex << (uint)footer << "\n";
    }
};

int main() {
    vector<byte> control = {0x7e, 0x10};
    vector<byte> addressing = {0x7e, 0x10};
    vector<byte> length = {4};
    vector<byte> packet = {0x7e, 0x10, 0x7e, 0x10};

    Frame frame(control, addressing, length, packet);

    frame.print();
}