#include <stdlib.h>
#include "debug.hpp"
#include "dll_test.hpp"
#include "mem.hpp"

int main() {
    init_uart0();   // init uart
    _delay_ms(100); // delay for uart to initialize properly
    #ifndef WINDOWS
        put_str("--------------------------------------------------------\r\n");
    #endif

    if (dll_test()) {
        put_str("Failed :(\r\n");
        return 1;
    } else {
        put_str("Success :D\r\n");
        return 0;
    }
    
    // #define NUM_TESTS 1
    // for (uint16_t i = 0; i < NUM_TESTS; i++) {
    //     if (dll_test() != 0) {
    //         put_str("Error: Test ");
    //         put_uint16(i + 1);
    //         put_str(" failed :(\r\n");
    //         return 1;
    //     } else if ((i + 1) % (NUM_TESTS/10) == 0) {
    //         put_uint16(i + 1);
    //         put_str(" tests passed\r\n");
    //     }
    // }
    // put_str("All test passed!\r\n");

    // Buffer a, b;
    // send(dll, a);
    // send(dll, a);
    // send(dll, a);
    // b = receive(dll);
    // check(a, b);
    // deallocate(a);
    // deallocate(b);
}