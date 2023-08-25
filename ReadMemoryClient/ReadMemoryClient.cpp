#include <iostream>
#include <Windows.h>
#include "DriverCommunication.hpp"

/*
    we'll use a queue system for the requests and retrieving the info from the driver
    struct of the driver_init_buffer    : | PATTERN_1 | PATTERN_2 | FIRST_REQUEST_TABLE_ENTRY | bit (1 if driver has found the buffer and is ready to initiate r/w operations, 0 otherwise)
    struct of the driver_request_buffer : | next buffer addr | remaining size | data, each data entry is DWORD64 + SIZE_T(address + size of read)
*/

int main()
{
    DriverCommunication com;
    auto test = com.read_next_entry_from_driver<int>();
    std::cout << "Hello World!\n";
}

