#include "dll.hpp"
#include "mem.hpp"
#include <string.h>

#ifdef DEBUG_MEM_ELABORATE
    #define allocate(x, ...) put_str(#x); put_str(": "); allocate(x, ##__VA_ARGS__)
    #define reallocate(x, ...) put_str(#x); put_str(": "); reallocate(x,##__VA_ARGS__)
    #define deallocate(x, ...) put_str(#x); put_str(": "); deallocate(x, ##__VA_ARGS__)
#endif

void DLL::send(uint8_t* packet, uint8_t packet_length, uint8_t destination_MAC_address) {
    uint8_t last_split_packet_num, last_split_packet_length;

    // Split packets
    if (packet_length > MAX_PACKET_LENGTH) {
        // Determine if there is a remainder split packet
        bool remainder_split_packet = packet_length % MAX_PACKET_LENGTH;

        // Calculate the last split packet number
        last_split_packet_num = (packet_length / MAX_PACKET_LENGTH) - 1 + remainder_split_packet;

        // Calculate the length of the last split packet
        last_split_packet_length = packet_length - (MAX_PACKET_LENGTH * last_split_packet_num);
    }
    
    // Single packet
    else {
        last_split_packet_num = 0;
        last_split_packet_length = packet_length;
    }

    // For each split packet frame
    for (uint8_t split_packet_num = 0; split_packet_num <= last_split_packet_num; split_packet_num++) {
        #ifdef DEBUG_DLL
            // Print SENDING FRAME
            put_str("\r\nSENDING FRAME\r\n");
        #endif

        // Insert control bytes
        frame.control[0] = split_packet_num;
        frame.control[1] = last_split_packet_num;
        #ifdef PRINT_STEPS
            if (last_split_packet_num != 0) {
                put_str("Frame sending as split packet "); put_uint8(frame.control[0]); put_ch('/'); put_uint8(frame.control[1]); put_str("\r\n"); 
            }
        #endif

        // Insert addressing bytes
        frame.addressing[0] = DEVICE_MAC_ADDRESS;
        #ifdef PRINT_STEPS
            put_str("Device MAC address:      "); put_hex(frame.addressing[0]); put_str("\r\n");
        #endif
        frame.addressing[1] = destination_MAC_address;
        #ifdef PRINT_STEPS
            put_str("Destination MAC address: "); put_hex(frame.addressing[1]); put_str("\r\n");
        #endif

        // Insert length and allocate NET packet
        if (split_packet_num < last_split_packet_num) {
            // Not last split packet
            allocate(frame.net_packet, frame.length, MAX_PACKET_LENGTH);
        } else {
            // Last split packet or single packet
            allocate(frame.net_packet, frame.length, last_split_packet_length);
        }
        #ifdef PRINT_STEPS
            put_str("NET packet length: "); put_uint8(frame.length); put_str("\r\n");
        #endif

        // Insert NET packet
        for (uint8_t i = 0; i < frame.length; i++) {
            frame.net_packet[i] = packet[(split_packet_num * MAX_PACKET_LENGTH) + i]; 
        }
        #ifdef PRINT_STEPS
            put_str("NET packet: "); print(frame.net_packet, frame.length);
        #endif

        // Calculate CRC
        #ifdef PRINT_STEPS
            put_str("Calculating CRC...\r\n");
        #endif
        uint16_t crc = calculate_crc();

        // Insert CRC
        frame.checksum[0] = (crc & 0xFF00) >> 8;
        frame.checksum[1] = (crc & 0x00FF);
        #ifdef PRINT_STEPS
            put_str("CRC: "); put_hex(frame.checksum[0]); put_ch(' '); put_hex(frame.checksum[1]); put_str("\r\n");
        #endif

        #ifdef PRINT_FRAMES
            // Print the frame
            put_str("Frame:\r\n");
            print(frame);
        #endif

        // Stuff bytes
        #ifdef PRINT_STEPS
            put_str("Stuffing bytes...\r\n");
        #endif
        byte_stuff(); // Allocates stuffed_frame

        #ifdef PRINT_FRAMES
            // Print the stuffed frame
            put_str("Stuffed frame:\r\n"); print(stuffed_frame, stuffed_frame_length);
        #endif

        // Deallocate frame.net_packet
        deallocate(frame.net_packet, frame.length);

        #ifdef RANDOM_ERRORS
            // Insert random errors in 1/ERROR_RARITY frames
            uint8_t random_number = rand() % ERROR_RARITY;
            if (random_number == 0) {

                random_number = rand() % DROP_RARITY;

                if (random_number == 0) {
                    #ifdef DEBUG_RANDOM_ERRORS
                        put_str("Random error: Dropping frame\r\n");
                    #endif        
                                
                    // Deallocate stuffed_frame
                    deallocate(stuffed_frame, stuffed_frame_length);

                    // Drop frame (continue to next frame)
                    continue;
                } else {
                    // Select a random number of bytes to change
                    uint8_t num_changes = rand() % stuffed_frame_length;
                    
                    #ifdef DEBUG_RANDOM_ERRORS
                        put_str("Random error: Changing bytes\r\n");
                    #endif

                    // Change bytes
                    for (uint8_t _ = 0; _ < num_changes; _++) {
                        // Select a random byte in stuffed frame and change it to a random value
                        uint8_t byte_num = rand() % stuffed_frame_length;
                        stuffed_frame[byte_num] = rand() % 0xFF + 1;
                    }
                }
            }
        #endif

        // Pass the frame...
        #ifdef VIRTUAL_RECEIVER
            // ...to virtual DLL receiver
            #ifdef DEBUG_DLL
                put_str("Passing frame to virtual DLL receiver\r\n");
            #endif
            receive(stuffed_frame, stuffed_frame_length);
        #else
            // ...to PHY
            #ifdef DEBUG_DLL
                put_str("Passing frame to PHY\r\n");
            #endif
            phy->send(stuffed_frame, stuffed_frame_length);
            
            // Deallocate stuffed_frame
            deallocate(stuffed_frame, stuffed_frame_length);
        #endif

    }
}

void DLL::receive(uint8_t* received_frame, uint8_t received_frame_length) {
    #ifdef DEBUG_DLL
        // Print RECEIVING FRAME
        put_str("\r\nRECEIVING FRAME\r\n");
    #endif

    // Copy received_frame into stuffed_frame
    #ifndef VIRTUAL_RECEIVER
        allocate(stuffed_frame, stuffed_frame_length, received_frame_length);
        memcpy(stuffed_frame, received_frame, stuffed_frame_length);
    #endif

    #ifdef PRINT_FRAMES
        // Print the stuffed frame
        put_str("Stuffed frame:\r\n");
        print(stuffed_frame, stuffed_frame_length);
    #endif

    // De-byte-stuff the stuffed frame
    #ifdef PRINT_STEPS
        put_str("Destuffing bytes...\r\n");
    #endif
    de_byte_stuff(); // Allocates frame.net_packet

    // Deallocate stuffed_frame
    deallocate(stuffed_frame, stuffed_frame_length);

    #ifdef PRINT_FRAMES
        // Print the frame
        put_str("Frame:\r\n");
        print(frame);
    #endif

    // Check CRC
    #ifdef PRINT_STEPS
        put_str("Checking CRC...\r\n"); 
    #endif
    bool error_in_frame = check_crc();

    // Errror in frame handling
    if (error_in_frame == true) {
        #ifdef PRINT_STEPS
            put_str("CRC check failed\r\n"); 
        #endif

        // Split packet frame
        if (frame.control[1] != 0) {
            #ifdef DEBUG_DLL
                put_str("Dropping frame: Error detected in split packet frame\r\n");
            #endif

            if (reconstructed_packet_length > 0) {
                // Drop previously received reconstructed packet
                #ifdef PRINT_STEPS
                    put_str("Dropping partially reconstructed packet\r\n"); 
                #endif
                deallocate(reconstructed_packet, reconstructed_packet_length);
            }

            // Set error in split packet sequence flag to true
            error_in_split_packet_sequence = true;
        }
        
        #ifdef DEBUG_DLL
            // Single packet error
            else {
                put_str("Dropping frame: Error detected in frame\r\n");
            }
        #endif

        // Drop frame
        deallocate(frame.net_packet, frame.length);
        return;
    }
    #ifdef PRINT_STEPS
        put_str("CRC check passed\r\n"); 
    #endif

    // Check destination MAC address matches device's or is in broadcast mode
    #ifdef PRINT_STEPS
        put_str("Checking destination address...\r\n");
        put_str("Destination MAC address: "); put_hex(frame.addressing[1]); put_str("\r\n");
        put_str("Device MAC address:      "); put_hex(DEVICE_MAC_ADDRESS); put_str("\r\n");
    #endif
    if (frame.addressing[1] != DEVICE_MAC_ADDRESS and frame.addressing[1] != 0xFF) {
        #ifdef PRINT_STEPS
            put_str("Destination address check failed\r\n"); 
        #endif

        // Drop frame
        #ifdef DEBUG_DLL
            put_str("Dropping frame: Destination address does not match device\r\n");
        #endif
        deallocate(frame.net_packet, frame.length);
        return;
    }
    #ifdef PRINT_STEPS
        put_str("Destination address check passed\r\n"); 
    #endif
    
    if (expected_split_packet_num == 0) {
        expected_last_split_packet_num = frame.control[1];
    }

    // Check split packet numbers
    #ifdef PRINT_STEPS
        put_str("Checking split packet numbers...\r\n");
        put_str("Expected split packet number: "); put_hex(expected_split_packet_num); put_str("\r\n");
        put_str("Received split packet number: "); put_hex(frame.control[0]); put_str("\r\n");
        put_str("Expected last split packet number: "); put_hex(expected_last_split_packet_num); put_str("\r\n");
        put_str("Received last split packet number: "); put_hex(frame.control[1]); put_str("\r\n");
    #endif
    if (frame.control[0] != expected_split_packet_num or frame.control[1] != expected_last_split_packet_num) {
        // If frame is part of new split packet sequence
        if (frame.control[1] != expected_last_split_packet_num) {
            // Set new last split packet number
            expected_last_split_packet_num = frame.control[1];
        }

        // If first split packet or single packet
        if (frame.control[0] == 0) {
            // Reset error in split packet sequence flag
            error_in_split_packet_sequence = false;

            // If expected to receive a continuing split packet
            if (expected_split_packet_num != 0) {
                #ifdef DEBUG_DLL
                    put_str("Warning: Previous split packet sequence lost, processing new split packet sequence\r\n");
                #endif

                // Reset expected split packet number
                expected_split_packet_num = 0;
            }
        }

        // If not first split packet
        else {
            // Set error in split packet sequence flag true
            error_in_split_packet_sequence = true;

            // Increment expected split packet number
            expected_split_packet_num++;
        }
    }
    #ifdef PRINT_STEPS
        put_str("Split packet numbers check passed\r\n");
    #endif

    // Error in split packet sequence handling
    if (error_in_split_packet_sequence == true) {
        // Reset error flag to false when last split packet is reached
        if (frame.control[0] == frame.control[1]) {
            error_in_split_packet_sequence = false;
        }

        // Drop frame
        #ifdef DEBUG_DLL
            put_str("Dropping frame: Error detected in split packet sequence\r\n");
        #endif
        deallocate(frame.net_packet, frame.length);
        return;
    }

    // Process a single packet frame
    if (frame.control[1] == 0) {
        #ifdef DEBUG_DLL
            // Print the received packet
            put_str("Received packet: "); print(frame.net_packet, frame.length);
        #endif

        #ifdef VIRTUAL_RECEIVER
            // Store the recieved packet
            allocate(received_packet, received_packet_length, frame.length);
            memcpy(received_packet, frame.net_packet, received_packet_length);
        #else
            // Pass the packet to NET
            #ifdef DEBUG_DLL
                put_str("Passing packet to NET\r\n");
            #endif
            net->receive(frame.net_packet, frame.length, frame.addressing[0]);
        #endif
        #ifdef DEBUG_DLL
            put_str("\r\n");
        #endif
    }
    
    // Process a split packet frame
    else {
        #ifdef PRINT_STEPS
            // Print the split packet number
            put_str("Frame received as part of split packet "); put_uint8(frame.control[0]); put_ch('/'); put_uint8(frame.control[1]); put_str("\r\n"); 
        #endif

        // First split packet
        if (frame.control[0] == 0) {
            #ifdef PRINT_STEPS
                // Print the split packet
                put_str("Split packet: "); print(frame.net_packet, frame.length);
                put_str("Storing split packet...\r\n"); 
            #endif
            
            // Copy frame.net_packet into reconstructed_packet
            allocate(reconstructed_packet, reconstructed_packet_length, frame.length);
            memcpy(reconstructed_packet, frame.net_packet, reconstructed_packet_length);

            #ifdef DEBUG_DLL
                // Print the partially reconstructed packet
                put_str("Partially reconstructed packet: "); print(reconstructed_packet, reconstructed_packet_length);
            #endif

            // Increment expected split packet number
            expected_split_packet_num++;

        // n'th split packet
        } else {
            #ifdef PRINT_STEPS
                // Print the split packet
                put_str("Split packet: "); print(frame.net_packet, frame.length);
                put_str("Storing split packet...\r\n");
            #endif

            // Append frame.net_packet onto reconstructed_packet
            reallocate(reconstructed_packet, reconstructed_packet_length, reconstructed_packet_length + frame.length);
            memcpy(&reconstructed_packet[reconstructed_packet_length - frame.length], frame.net_packet, frame.length);

            #ifdef DEBUG_DLL
                // Print the partially or fully reconstructed packet
                if (frame.control[0] < frame.control[1]) {
                    put_str("Partially");
                } else {
                    put_str("Fully");
                }
                put_str(" reconstructed packet: "); print(reconstructed_packet, reconstructed_packet_length);
            #endif

            // Increment expected split packet number
            expected_split_packet_num++;

            // Last split packet
            if (frame.control[0] == frame.control[1]) {
                // Reset expected split packet number to 0
                expected_split_packet_num = 0;

                #ifdef VIRTUAL_RECEIVER
                    allocate(received_packet, received_packet_length, reconstructed_packet_length);
                    memcpy(received_packet, reconstructed_packet, reconstructed_packet_length);
                #else
                    #ifdef DEBUG_DLL
                        put_str("Passing packet to NET...\r\n");
                    #endif
                    net->receive(reconstructed_packet, reconstructed_packet_length, frame.addressing[0]);
                #endif
                #ifdef DEBUG_DLL
                    put_str("\r\n");
                #endif

                // Deallocate reconstructed_packet
                deallocate(reconstructed_packet, reconstructed_packet_length);
            }
        }
    }

    // Deallocate frame.net_packet
    deallocate(frame.net_packet, frame.length);
}

void DLL::byte_stuff() {
    uint8_t message_length;
    uint8_t* message = NULL;
    allocate(message, message_length, 2 + 2 + 1 + frame.length + 2);
    message[0] = frame.control[0];
    message[1] = frame.control[1];
    message[2] = frame.addressing[0];
    message[3] = frame.addressing[1];
    message[4] = frame.length;
    for (uint8_t i = 0; i < frame.length; i++) {
        message[5 + i] = frame.net_packet[i];
    }
    message[message_length - 2] = frame.checksum[0];
    message[message_length - 1] = frame.checksum[1];
    // print(message, message_length);

    for (uint8_t i = 0; i < message_length; i++) {
        // Detect FLAG or ESC
        if (message[i] == FLAG or message[i] == ESC) {
            #ifdef PRINT_BYTE_STUFFING
                print(message, message_length);
                put_str("Escaping ");
                if (message[i] == FLAG) {
                    put_str("flag");
                } else {
                    put_str("escape");
                }
                put_str(" byte detected at byte "); put_uint8(i+1); put_str("...\r\n");
            #endif
            // Increment length of message
            reallocate(message, message_length, message_length + 1);
            // print(message, message_length);
            // Shift bytes after i right
            uint8_t temp[message_length - i];
            memcpy(temp, &message[i], message_length - i);
            memcpy(&message[i + 1], temp, message_length - i);
            // Insert ESC at i
            message[i] = ESC;
            // XOR escaped byte
            // message[i + 1] ^= 0x20;
            // Skip escaped (next) byte
            i++;
            // print(message, message_length);
            #ifdef PRINT_BYTE_STUFFING
                print(message, message_length);
            #endif
        }
    }

    allocate(stuffed_frame, stuffed_frame_length, 1 + message_length + 1);
    stuffed_frame[0] = FLAG;
    memcpy(&stuffed_frame[1], message, message_length);
    stuffed_frame[stuffed_frame_length - 1] = FLAG;

    deallocate(message, message_length);
}

void DLL::de_byte_stuff() {
    uint8_t message_length;
    uint8_t* message = NULL;
    allocate(message, message_length, stuffed_frame_length - 2);
    memcpy(message, &stuffed_frame[1], message_length);
    
    for (uint8_t i = 0; i < message_length; i++) {
        if (message[i] == ESC) {
            #ifdef PRINT_BYTE_STUFFING
                print(message, message_length);
                put_str("Removing escape byte detected at byte "); put_uint8(i+1); put_str("...\r\n"); 
            #endif
            // Shift bytes after i left
            memcpy(&message[i], &message[i + 1], message_length - i);
            // XOR de-escaped byte
            // message[i] ^= 0x20;
            // Decrement message length
            reallocate(message, message_length, message_length - 1);
            #ifdef PRINT_BYTE_STUFFING
                print(message, message_length);
            #endif
        }
    }

    frame.control[0] = message[0];
    frame.control[1] = message[1];
    frame.addressing[0] = message[2];
    frame.addressing[1] = message[3];
    allocate(frame.net_packet, frame.length, message[4]);
    memcpy(frame.net_packet, &message[5], frame.length);
    frame.checksum[0] = message[message_length - 2];
    frame.checksum[1] = message[message_length - 1];

    deallocate(message, message_length);
}

uint16_t DLL::calculate_crc() {
    // Copy frame contents into a byte array
    uint8_t message_length = 2 + 2 + 1 + frame.length;
    uint8_t message[message_length];
    message[0] = frame.control[0];
    message[1] = frame.control[1];
    message[2] = frame.addressing[0];
    message[3] = frame.addressing[1];
    message[4] = frame.length;
    for (uint8_t i = 0; i < frame.length; i++) {
        message[5 + i] = frame.net_packet[i];
    }
    #ifdef PRINT_STEPS
        put_str("Message: "); print(message, message_length);
        put_str("Polynomial: "); put_hex(POLYNOMIAL); put_str("\r\n");
    #endif
    // Initialize the value of the CRC to 0
    uint16_t crc = 0;
    // Perform modulo-2 division, a byte at a time.
    for (int byte = 0; byte < message_length; byte++) {
        // Bring the next byte into the crc.
        crc ^= message[byte] << 8;
        // Perform modulo-2 division, a bit at a time.
        for (uint8_t bit = 8; bit > 0; bit--) {
            // Try to divide the current data bit.
            if (crc & (1 << 15)) {
                crc = (crc << 1) ^ POLYNOMIAL;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

bool DLL::check_crc() {
    uint16_t crc = calculate_crc();
    uint8_t expected_crc[2];
    expected_crc[0] = (crc & 0xFF00) >> 8;
    expected_crc[1] = (crc & 0x00FF);
    #ifdef PRINT_STEPS
        put_str("Expected CRC: "); put_hex(expected_crc[0]); put_ch(' '); put_hex(expected_crc[1]); put_str("\r\n");
        put_str("Received CRC: "); put_hex(frame.checksum[0]); put_ch(' '); put_hex(frame.checksum[1]); put_str("\r\n");
    #endif
    if (frame.checksum[0] != expected_crc[0] or frame.checksum[1] != expected_crc[1]) {
        return 1;
    } else {
        return 0;
    }
}

Frame::Frame() {
    header = FLAG;
    length = 0;
    net_packet = NULL;
    footer = FLAG;
}

DLL::DLL() {
    stuffed_frame = NULL;
    stuffed_frame_length = 0;
    reconstructed_packet = NULL;
    reconstructed_packet_length = 0;
    #ifdef VIRTUAL_RECEIVER
        received_packet = NULL;
        received_packet_length = 0;
    #endif
    error_in_split_packet_sequence = false;
    expected_split_packet_num = 0;
    expected_last_split_packet_num = 0;
}

uint8_t max(uint8_t a, uint8_t b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

void print(Frame frame) {    
    /*
    +--------+-----------+------------+--------+---------------------+------------+--------+
    | Header |  Control  | Addressing | Length |      NET Packet     |  Checksum  | Footer |
    +--------+-----------+------------+--------+---------------------+------------+--------+
    |  0x7d  | 0x7d 0x7e | 0x7d  0x7e |  0x04  | 0x7d 0x7e 0x7d 0x7e | 0x7d  0x7e |  0x7d  |
    +--------+-----------+------------+--------+---------------------+------------+--------+
    */
    uint8_t num_dashes = max(12, 1 + frame.length*5);
    uint8_t num_spaces = num_dashes - 10;
    uint8_t extra_space = num_dashes % 2;
    put_str("+--------+-----------+------------+--------+");
    if (frame.length > 0) {
        for (uint8_t i = 0; i < num_dashes; i++) {
            put_ch('-');
        }
        put_ch('+');
    }
    put_str("------------+--------+\r\n");
    put_str("| Header |  Control  | Addressing | Length |");
    if (frame.length > 0) {
        for (uint8_t i = 0; i < num_spaces/2 + extra_space; i++) {
            put_ch(' ');
        }
        put_str("NET Packet");
        for (uint8_t i = 0; i < num_spaces/2; i++) {
            put_ch(' ');
        }
        put_ch('|');
    }
    put_str("  Checksum  | Footer |\r\n");
    put_str("+--------+-----------+------------+--------+");
    if (frame.length > 0) {
        for (uint8_t i = 0; i < num_dashes; i++) {
            put_ch('-');
        }
        put_ch('+');
    }
    put_str("------------+--------+\r\n");
    put_str("|  ");
    put_hex(frame.header);
    put_str("  | ");
    put_hex(frame.control[0]);
    put_ch(' ');
    put_hex(frame.control[1]);
    put_str(" | ");
    put_hex(frame.addressing[0]);
    put_str("  ");
    put_hex(frame.addressing[1]);
    put_str(" |  ");
    put_hex(frame.length);
    put_str("  | ");
    if (frame.length > 2) {
        for (uint8_t i = 0; i < frame.length; i++) {
            put_hex(frame.net_packet[i]);
            put_ch(' ');
        }
        put_str("| ");
    } else if (frame.length == 2) {
        put_hex(frame.net_packet[0]);
        put_str("  ");
        put_hex(frame.net_packet[1]);
        put_str(" | ");
    } else if (frame.length == 1) {
        put_str("   ");
        put_hex(frame.net_packet[0]);
        put_str("    | ");
    }
    put_hex(frame.checksum[0]);
    put_str("  ");
    put_hex(frame.checksum[1]);
    put_str(" |  ");
    put_hex(frame.footer);
    put_str("  |\r\n");
    put_str("+--------+-----------+------------+--------+");
    if (frame.length > 0) {
        for (uint8_t i = 0; i < num_dashes; i++) {
            put_ch('-');
        }
        put_ch('+');
    }
    put_str("------------+--------+\r\n");
}

void print(uint8_t* buffer, uint8_t buffer_length) {
    for (uint8_t byte = 0; byte < buffer_length; byte++) {
        put_hex(buffer[byte]);
        put_ch(' ');
    }
    put_str("\r\n");
}

bool DLL::test(uint8_t max_packet_length, PACKET_LENGTH_OPTIONS packet_length_option, PACKET_DATA_OPTIONS packet_data_option, DESTINATION_MAC_ADDRESS_OPTIONS destination_MAC_address_option) {
    // Set the packet length
    uint8_t packet_length;
    switch (packet_length_option) {
    // Random packet length between 1 and max_packet_length
    case RANDOM:
        packet_length = rand() % max_packet_length + 1;
        break;
    // Fixed packet length
    case FIXED:
        packet_length = max_packet_length;
        break;
    default:
        put_str("Error: Invalid length option in test\r\n");
        exit(1);
    }
    uint8_t packet[packet_length];
    
    // Initialise data to send
    for (uint16_t byte = 0; byte < packet_length; byte++) {
        switch (packet_data_option) {
        // All possible values
        case ALL:
            packet[byte] = rand() % 0x100;
            break;
        // FLAG bytes only
        case FLAG_ONLY:
            packet[byte] = FLAG;
            break;
        // ESC bytes only
        case ESC_ONLY:
            packet[byte] = ESC;
            break;
        // FLAG and ESC bytes only
        case FLAG_AND_ESC_ONLY:
            packet[byte] = rand() % 2 + FLAG;
            break;
        // Sequential numbering
        case SEQUENTIAL:
            packet[byte] = byte;
            break;
        default:
            put_str("Error: Invalid data option in test\r\n");
            exit(1);
        }
    }

    // Set destination MAC address
    uint8_t destination_MAC_address;
    switch (destination_MAC_address_option)
    {
    // Device address
    case DEVICE:
        destination_MAC_address = DEVICE_MAC_ADDRESS;
        break;
    // Broadcast address
    case BROADCAST:
        destination_MAC_address = 0xFF;
        break;
    // Always incorrect address
    case WRONG:
        destination_MAC_address = DEVICE_MAC_ADDRESS + 1;
        break;
    default:
        put_str("Error: Invalid address option in test\r\n");
        exit(1);
    }

    // Send and receive packet
    #ifdef DEBUG_TEST
        put_str("Sending packet:  "); print(packet, packet_length);
    #endif
    send(packet, packet_length, destination_MAC_address);
    #ifdef DEBUG_TEST
        put_str("Received packet: "); print(received_packet, received_packet_length);
        #ifdef DEBUG_DLL
            put_str("Sent     packet: "); print(packet, packet_length);
        #endif
    #endif

    // Check sent and received packet lengths match
    if (received_packet_length != packet_length) {
        #ifdef DEBUG_DLL
            put_str("\r\n");
        #endif
        put_str("Error: Packet lengths do not match\r\n");
        put_str("Sent     packet length: "); put_uint8(packet_length); put_str("\r\n");
        put_str("Received packet length: "); put_uint8(received_packet_length); put_str("\r\n");
        return 1;
    }

    // Check sent and received packet contents matches
    for (uint8_t byte = 0; byte < packet_length; byte++) {
        if (received_packet[byte] != packet[byte]) {
            #ifdef DEBUG_DLL
                put_str("\r\n");
            #endif
            put_str("Error: Packet contents do not match\r\n");
            return 1;
        }
    }

    #ifdef DEBUG_DLL
        put_str("Sent and received packets match\r\n");
    #endif

    // Deallocate received packet
    deallocate(received_packet, received_packet_length);

    // Check for memory leaks
    if (mem_leak()) {
        put_str("Error: Memory leak\r\n");
        print_mem_use();
        return 1;
    }

    return 0;
}