#include "dll.hpp"

// Test options
#define NUMBER_OF_TESTS 1000
#define MAXIMUM_PACKET_LENGTH 255
#define PACKET_LENGTH_OPTION RANDOM
#define PACKET_DATA_OPTION ALL
#define DESTINATION_MAC_ADDRESS_OPTION DEVICE

int main() {
    DLL dll;

    uint16_t num_tests_failed = 0;

    for (uint16_t test_num = 0; test_num < NUMBER_OF_TESTS; test_num++) {
        // Tests sending and receiving a packet configured by these options
        bool error = dll.test(MAXIMUM_PACKET_LENGTH, PACKET_LENGTH_OPTION, PACKET_DATA_OPTION, DESTINATION_MAC_ADDRESS_OPTION);

        put_str("Test "); put_uint16(test_num + 1); 
        if (error == true) {
            put_str(" failed\r\n");
            num_tests_failed++;
        } else {
            put_str(" passed\r\n");
        }
        // put_str("\r\n");
    }

    put_str("-------\r\n");
    put_str("Results\r\n");
    put_str("-------\r\n");
    put_str("FAIL: "); put_uint16(num_tests_failed); put_str(", PASS: "); put_uint16(NUMBER_OF_TESTS - num_tests_failed); put_str("\r\n");
}