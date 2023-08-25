#include "./DriverCommunication.hpp"

DriverCommunication::DriverCommunication() {
	
	//initialize the initialization buffer
	DWORD64 pattern_1 = PATTERN_1;
	DWORD64 pattern_2 = PATTERN_2;

	DWORD64 initialization_buffer = reinterpret_cast<DWORD64>(VirtualAlloc(NULL, VIRTUAL_PAGE_SIZE, MEM_COMMIT, PAGE_READWRITE)); //data to initalize requests with the driver
	std::memcpy(&initialization_buffer, &pattern_1, sizeof(DWORD64));
	std::memcpy(reinterpret_cast<PVOID>(initialization_buffer + sizeof(DWORD64)), &pattern_2, sizeof(DWORD64)); //copying both the patterns into buffer


	//creating the first recurring reads buffer
	DWORD64 first_recurring_reads_buffer = reinterpret_cast<DWORD64>(VirtualAlloc(NULL, REGISTER_RECURRING_READS_BUFFER_SIZE, MEM_COMMIT, PAGE_READWRITE)); //data to initalize requests with the driver
	std::memcpy(reinterpret_cast<PVOID>(initialization_buffer + 2*sizeof(DWORD64)), &first_recurring_reads_buffer, sizeof(DWORD64)); //copying the first request buffer to the initialization buffer

	SIZE_T default_init_buf_size = REGISTER_RECURRING_READS_BUFFER_REMAINING_BYTES_AFTER_INITIALIZATION;
	std::memcpy(reinterpret_cast<LPVOID>(GET_BUFFER_REMAINING_BYTES_ADDRESS(first_recurring_reads_buffer)), &default_init_buf_size, sizeof(SIZE_T));

	//store the recurring requests into our buffer into our vector
	this->recurring_requests_buffers.push_back(first_recurring_reads_buffer);
}

DriverCommunication::~DriverCommunication() {
	for (DWORD64 req_buf_addr : this->recurring_requests_buffers)
		VirtualFree(&req_buf_addr, NULL, MEM_RELEASE);

}

DWORD64 DriverCommunication::register_new_recurring_read(RecurringReadRequest request) {
	static BYTE zero_out_recurring_entry[RECURRING_READ_ENTRY_SIZE]; //static variables set to 0 by default
	for (DWORD64 recurring_buffer_addr : this->recurring_requests_buffers) {
		SIZE_T remaining_bytes = *reinterpret_cast<SIZE_T*>(GET_BUFFER_REMAINING_BYTES_ADDRESS(recurring_buffer_addr));
		if (remaining_bytes < RECURRING_READ_ENTRY_SIZE)
			continue;

		for (unsigned int i = RECURRING_READS_BUFFER_ENTRY_START; i < REGISTER_RECURRING_READS_BUFFER_SIZE; i += RECURRING_READ_ENTRY_SIZE) {
			if (!memcmp(zero_out_recurring_entry, reinterpret_cast<LPVOID>(recurring_buffer_addr + i), RECURRING_READ_ENTRY_SIZE)) {
				*reinterpret_cast<RecurringReadRequest*>(recurring_buffer_addr + i) = request;
				*reinterpret_cast<SIZE_T*>(GET_BUFFER_REMAINING_BYTES_ADDRESS(recurring_buffer_addr)) += RECURRING_READ_ENTRY_SIZE;
				return reinterpret_cast<DWORD64>(recurring_buffer_addr + i + sizeof(RecurringReadRequest));
			}
		}
	}

	DWORD64 new_recurring_read_buffer = reinterpret_cast<DWORD64>(VirtualAlloc(NULL, REGISTER_RECURRING_READS_BUFFER_SIZE, MEM_COMMIT, PAGE_READWRITE));
	if (!new_recurring_read_buffer)
		return NULL;

	std::memcpy(&new_recurring_read_buffer, &this->recurring_requests_buffers.front(), sizeof(DWORD64));

	SIZE_T default_init_buf_size = REGISTER_RECURRING_READS_BUFFER_REMAINING_BYTES_AFTER_INITIALIZATION;
	std::memcpy(reinterpret_cast<LPVOID>(GET_BUFFER_REMAINING_BYTES_ADDRESS(new_recurring_read_buffer)), &default_init_buf_size, sizeof(SIZE_T));
	
	*reinterpret_cast<RecurringReadRequest*>(new_recurring_read_buffer + RECURRING_READS_BUFFER_ENTRY_START) = request;
	*reinterpret_cast<SIZE_T*>(GET_BUFFER_REMAINING_BYTES_ADDRESS(new_recurring_read_buffer)) += RECURRING_READ_ENTRY_SIZE;
	return reinterpret_cast<DWORD64>(new_recurring_read_buffer + RECURRING_READS_BUFFER_ENTRY_START);
}

bool DriverCommunication::unregister_recurring_read(DWORD64 entry_addr) {
	static RecurringReadRequest zero_out_recurring_read_request = { 0, 0, 0 };
	for (DWORD64 recurring_buffer_addr : this->recurring_requests_buffers) {
		if (entry_addr < recurring_buffer_addr || entry_addr > recurring_buffer_addr + REGISTER_RECURRING_READS_BUFFER_SIZE)
			continue;

		*reinterpret_cast<RecurringReadRequest*>(entry_addr) = zero_out_recurring_read_request;
		*reinterpret_cast<DWORD64*>(GET_DATA_BUFFER_ADDRESS_FROM_ENTRY(entry_addr)) = 0;
		*reinterpret_cast<SIZE_T*>(GET_BUFFER_REMAINING_BYTES_ADDRESS(recurring_buffer_addr)) -= RECURRING_READ_ENTRY_SIZE;
		return true;
	}
	
	return false;
}
