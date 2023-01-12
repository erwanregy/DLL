#include "dll.hpp"

// Test options
#define NUMBER_OF_TESTS 1
#define MAXIMUM_PACKET_LENGTH 24
#define PACKET_LENGTH_OPTION FIXED
#define PACKET_DATA_OPTION ALL
#define DESTINATION_MAC_ADDRESS_OPTION DEVICE

int main() {
    DLL dll;
    uint16_t number_of_failed_tests = 0;

    for (uint16_t test_number = 1; test_number <= NUMBER_OF_TESTS; test_number++) {
        // Tests sending and receiving a packet configured by these options
        bool error = dll.test(MAXIMUM_PACKET_LENGTH, PACKET_LENGTH_OPTION, PACKET_DATA_OPTION, DESTINATION_MAC_ADDRESS_OPTION);

        put_str("Test "); put_uint16(test_number); 
        if (error == true) {
            put_str(" failed\r\n");
            number_of_failed_tests++;
        } else {
            put_str(" passed\r\n");
        }
    }

    put_str("-------\r\n");
    put_str("Results\r\n");
    put_str("-------\r\n");
    put_str("PASS: "); put_uint16(NUMBER_OF_TESTS - number_of_failed_tests);  put_str(", ");
    put_str("FAIL: "); put_uint16(number_of_failed_tests); put_str("\r\n");
    put_uint16((NUMBER_OF_TESTS - number_of_failed_tests) * 100 / NUMBER_OF_TESTS); put_str("%% success rate\r\n");
}