#pragma once
#include <iostream>
#include <Windows.h>
#include <vector>
#include <queue>

#include "./common.hpp"

#define VIRTUAL_PAGE_SIZE 4096

#define REGISTER_RECURRING_READS_BUFFER_SIZE VIRTUAL_PAGE_SIZE
#define REGISTER_RECURRING_READS_BUFFER_REMAINING_BYTES_AFTER_INITIALIZATION (REGISTER_RECURRING_READS_BUFFER_SIZE - sizeof(DWORD64) - sizeof(unsigned long))


#define RECURRING_READS_BUFFER_ENTRY_START (sizeof(DWORD64) + sizeof(unsigned long))
#define RECURRING_READ_ENTRY_SIZE (sizeof(RecurringReadRequest) + sizeof(DWORD64)) //RecurringReadRequest, data addr

#define GET_DATA_BUFFER_ADDRESS_FROM_ENTRY(addr) (addr + sizeof(RecurringReadRequest)) //get the address returned by the driver
#define GET_BUFFER_REMAINING_BYTES_ADDRESS(buffer) (buffer + sizeof(DWORD64))
#define GET_NEXT_EMPTY_ENTRY_ADDRESS(buffer, remaining_bytes, buffer_size) (buffer + buffer_size - remaining_bytes)

/*
struct of register recurring read buffer: |next buffer addr | remaining bytes | data (each data entry: ReadRequest, interval, read data addr)
*/


struct RecurringReadRequest {
	DWORD64 read_addr;
	unsigned long read_size;
	unsigned int interval;
};


class DriverCommunication {
private:
	std::vector<DWORD64> recurring_requests_buffers; //will contain all the addresses to all the request buffers

public:
	DriverCommunication();
	~DriverCommunication();

	DWORD64 register_new_recurring_read(RecurringReadRequest request); //returns the start of the entry addr
	bool unregister_recurring_read(DWORD64 entry_addr);
	//bool add_new_read_request(ReadRequest request, BYTE tag); //tag is used so that the client can easily keep track of what read has been done
};