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
    auto test = com.register_new_recurring_read({ 500, sizeof(int), 100 });

    std::cout << "hi" << std::endl;
    std::cout << sizeof(RecurringReadRequest) << std::endl;

    while (true) {
        Sleep(5);
    }
}

