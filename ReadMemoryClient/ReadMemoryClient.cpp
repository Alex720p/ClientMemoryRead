#include <iostream>
#include <Windows.h>

#define PATTERN_1 0xAABBCCDDEEFFAABB
#define PATTERN_2 0xBBAAFFEEDDCCBBAA //will be used to find ReadMemoryClient


#define VIRTUAL_PAGE_SIZE 4096
#define DEFAULT_DRIVER_REQUEST_BUFFER_SIZE 4*VIRTUAL_PAGE_SIZE
#define DEFAULT_DRIVER_REQUEST_BUFFER_SIZE 4*VIRTUAL_PAGE_SIZE

/*
    we'll use a queue system for the requests and retrieving the info from the driver
    struct of the driver_init_buffer    : | PATTERN_1 | PATTERN_2 | FIRST_REQUEST_TABLE_ENTRY | bit (1 if driver has found the buffer and is ready to initiate r/w operations, 0 otherwise)
    struct of the driver_request_buffer : | next buffer addr | remaining size | data, each data entry is DWORD64 + SIZE_T(address + size of read)
*/

int main()
{
    HANDLE current_proc = GetCurrentProcess();
    DWORD current_proc_id = GetProcessId(current_proc);
    if (!current_proc_id) {
        std::cout << "Failed to initialize client" << std::endl;
        return 0;
    }

    DWORD64 pattern_1 = PATTERN_1;
    DWORD64 pattern_2 = PATTERN_2;

    DWORD64 driver_init_buffer = reinterpret_cast<DWORD64>(VirtualAlloc(NULL, VIRTUAL_PAGE_SIZE, MEM_COMMIT, PAGE_READWRITE)); //data to initalize requests with the driver
    std::memcpy(&driver_init_buffer, &pattern_1, sizeof(DWORD64));
    std::memcpy(&driver_init_buffer + sizeof(DWORD64), &pattern_2, sizeof(DWORD64)); //copying both the patterns into buffer

    DWORD64 driver_request_buffer = reinterpret_cast<DWORD64>(VirtualAlloc(NULL, VIRTUAL_PAGE_SIZE*4, MEM_COMMIT, PAGE_READWRITE)); //data to send to driver
    std::memcpy(&driver_init_buffer + 2 * sizeof(DWORD64), &driver_request_buffer, sizeof(DWORD64));

    DWORD64 driver_info_buffer = reinterpret_cast<DWORD64>(VirtualAlloc(NULL, VIRTUAL_PAGE_SIZE*4, MEM_COMMIT, PAGE_READWRITE)); //data received from driver
    std::cout << "Hello World!\n";
}

