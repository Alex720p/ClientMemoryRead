#include "./DriverCommunication.hpp"

DriverCommunication::DriverCommunication() {
	
	//initialize the initialization buffer
	DWORD64 pattern_1 = PATTERN_1;
	DWORD64 pattern_2 = PATTERN_2;

	DWORD64 initialization_buffer = reinterpret_cast<DWORD64>(VirtualAlloc(NULL, VIRTUAL_PAGE_SIZE, MEM_COMMIT, PAGE_READWRITE)); //data to initalize requests with the driver
	std::memcpy(&initialization_buffer, &pattern_1, sizeof(DWORD64));
	std::memcpy(reinterpret_cast<PVOID>(initialization_buffer + sizeof(DWORD64)), &pattern_2, sizeof(DWORD64)); //copying both the patterns into buffer

	DWORD64 first_request_buffer = reinterpret_cast<DWORD64>(VirtualAlloc(NULL, DEFAULT_DRIVER_REQUEST_BUFFER_SIZE, MEM_COMMIT, PAGE_READWRITE)); //data to initalize requests with the driver
	std::memcpy(reinterpret_cast<PVOID>(initialization_buffer + 2*sizeof(DWORD64)), &first_request_buffer, sizeof(DWORD64)); //copying the first request buffer to the initialization buffer

	//initialize the first request buffer
	DWORD64 second_request_buffer = reinterpret_cast<DWORD64>(VirtualAlloc(NULL, DEFAULT_DRIVER_REQUEST_BUFFER_SIZE, MEM_COMMIT, PAGE_READWRITE));
	std::memcpy(&first_request_buffer, &second_request_buffer, sizeof(DWORD64));

	SIZE_T default_init_req_size = DEFAULT_AFTER_INITIALIZATION_DRIVER_REQUEST_BUFFER_SIZE;
	std::memcpy(reinterpret_cast<PVOID>(first_request_buffer + sizeof(DWORD64)), &default_init_req_size, sizeof(SIZE_T));

	//initialize the second request buffer
	std::memcpy(&second_request_buffer, &first_request_buffer, sizeof(DWORD64));
	std::memcpy(reinterpret_cast<PVOID>(first_request_buffer + sizeof(DWORD64)), &default_init_req_size, sizeof(SIZE_T));

	//store the requests into our vector
	this->driver_request_info.push_back(first_request_buffer);
	this->driver_request_info.push_back(second_request_buffer);
}

DriverCommunication::~DriverCommunication() {
	for (DWORD64 req_buf_addr : this->driver_request_info)
		VirtualFree(&req_buf_addr, NULL, MEM_RELEASE);
}


bool DriverCommunication::allocate_new_request_buffer() {
	DWORD64 new_request_buffer = reinterpret_cast<DWORD64>(VirtualAlloc(NULL, DEFAULT_DRIVER_REQUEST_BUFFER_SIZE, MEM_COMMIT, PAGE_READWRITE));
	if (!new_request_buffer)
		return false;

	std::memcpy(&new_request_buffer, &this->driver_request_info.front(), sizeof(DWORD64));

	SIZE_T default_init_req_size = DEFAULT_AFTER_INITIALIZATION_DRIVER_REQUEST_BUFFER_SIZE;
	std::memcpy(reinterpret_cast<PVOID>(new_request_buffer + sizeof(DWORD64)), &default_init_req_size, sizeof(SIZE_T));
	return true;
}

bool DriverCommunication::add_new_read_request(ReadRequest request) {
	for (DWORD64 req_buf_addr : this->driver_request_info) {
		SIZE_T remaining_bytes = *reinterpret_cast<SIZE_T*>(GET_BUFFER_REMAINING_BYTES_ADDRESS(req_buf_addr));
		if (remaining_bytes < sizeof(ReadRequest))
			continue;

		*reinterpret_cast<ReadRequest*>(GET_BUFFER_WRITE_ADDRESS(req_buf_addr, DEFAULT_DRIVER_REQUEST_BUFFER_SIZE, remaining_bytes)) = request;
		this->scheduled_reads.push(request);
		return true;
	}
	
	//no more space in our read buffers
	if (!this->allocate_new_request_buffer())
		return false;

	DWORD64 new_req_buffer_addr = this->driver_request_info.back();
	*reinterpret_cast<ReadRequest*>(GET_BUFFER_WRITE_ADDRESS(new_req_buffer_addr, DEFAULT_DRIVER_REQUEST_BUFFER_SIZE, DEFAULT_AFTER_INITIALIZATION_DRIVER_REQUEST_BUFFER_SIZE)) = request;
	this->scheduled_reads.push(request);
	return true;
}