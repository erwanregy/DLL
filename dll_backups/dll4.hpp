#include <cstdint>

#define FLAG 0b01111110
#define ESC  0b00010000

class Section {
public:
    uint8_t  length;
    uint8_t* values;

    Section();
    Section(uint8_t value);
    Section(uint8_t length, uint8_t* values);
    ~Section();

    void byte_stuff();
    void de_byte_stuff();

    void print();
};

class Frame {
public:

    uint8_t header = FLAG;
    Section control;
    Section addressing;
    Section length;
    Section net_packet;
    Section checksum;
    uint8_t footer = FLAG;

    Frame(uint8_t* control, uint8_t* addressing, uint8_t length, uint8_t* net_packet);

    uint8_t* calc_crc();
    void byte_stuff();
    void de_byte_stuff();

    void print();
};