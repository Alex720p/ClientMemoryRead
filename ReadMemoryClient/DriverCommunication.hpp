#pragma once
#include <Windows.h>
#include <vector>
#include <queue>

#define PATTERN_1 0xAABBCCDDEEFFAABB
#define PATTERN_2 0xBBAAFFEEDDCCBBAA //will be used to find ReadMemoryClient

#define VIRTUAL_PAGE_SIZE 4096
#define DEFAULT_DRIVER_REQUEST_BUFFER_SIZE VIRTUAL_PAGE_SIZE
#define DEFAULT_DRIVER_READ_BUFFER_SIZE 4*VIRTUAL_PAGE_SIZE
#define DEFAULT_AFTER_INITIALIZATION_DRIVER_REQUEST_BUFFER_SIZE (DEFAULT_DRIVER_REQUEST_BUFFER_SIZE - sizeof(DWORD64) - sizeof(SIZE_T))

#define GET_BUFFER_REMAINING_BYTES_ADDRESS(buffer) (buffer + sizeof(DWORD64))
#define GET_BUFFER_WRITE_ADDRESS(buffer, buffer_size, remaining_bytes) (buffer + buffer_size - remaining_bytes)

/*
	we'll use a queue system for the requests and retrieving the info from the driver
	struct of the driver_init_buffer    : | PATTERN_1 | PATTERN_2 | FIRST_REQUEST_TABLE_ENTRY | FIRST_READS_TABLE_ENTRY  |
	struct of the driver_request_buffer : | next buffer addr | remaining size | data, each data entry is DWORD64 + SIZE_T(address + size of read)
	struct of the client_read_buffer : | next buffer addr | remaining size | data
*/

//todo: if a read is bigger than the buffer size(or remaining bytes), split the copy between multiple buffers
struct ReadRequest {
	DWORD64 read_addr;
	SIZE_T read_size;
};

class DriverCommunication {
private:
	std::vector<DWORD64> driver_request_info; //will contain all the addresses to all the request buffers
	std::queue<ReadRequest> scheduled_reads;
	DWORD64 first_read_buffer_entry;

	bool allocate_new_request_buffer();
public:
	DriverCommunication();
	~DriverCommunication();


	bool add_new_read_request(ReadRequest request);
};