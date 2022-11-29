#include "dll.hpp"

bool dll_test();

void send_packet(DLL& dll, Buffer& packet);

Buffer receive_packet(DLL& dll);

bool check_packets(Buffer sent, Buffer received);