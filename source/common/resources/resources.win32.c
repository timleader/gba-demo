
#include "resources.h"

#include "common/debug/debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

uint8_t* resource_base;
resource_entry_t* resources;
uint16_t resource_count;


HANDLE file_handle;
HANDLE file_mapping_handle;
char* real_data;

void resources_initialize()
{
	file_handle = CreateFile(
		"../data/package_7days.res", GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (file_handle == INVALID_HANDLE_VALUE)	///	this is a fatal error
	{
		DWORD error_code = GetLastError();
		exit(1);
	}

	DWORD high_size;
	DWORD low_size = GetFileSize(file_handle, &high_size);

	size_t mapping_size = high_size;
	mapping_size <<= 32;
	mapping_size |= low_size;

	file_mapping_handle = CreateFileMapping(
		file_handle, 0, PAGE_READONLY, high_size, low_size, 0); 

	if (file_mapping_handle == INVALID_HANDLE_VALUE) 
		return;

	real_data = (char*)(MapViewOfFile(
		file_mapping_handle, FILE_MAP_READ, 0, 0, mapping_size));

	if (!real_data) 
		return;

	resource_count = *((uint16_t*)real_data);
	resources = (real_data + 2);
	resource_base = real_data;

	debug_printf(DEBUG_LOG_INFO, "resources::initialized");
	for (uint16_t id = 0; id < resource_count; ++id)
	{
		debug_assert((resources[id].offset & 0x03) == 0, "resources::initialize resources not aligned");
		//debug_printf(DEBUG_LOG_INFO, "\tresources[%u].offset = %u", id, resources[id].offset);
	}
}

void resources_shutdown()
{
	if (real_data)
	{
		UnmapViewOfFile(real_data);
		real_data = NULL;
	}

	if (file_mapping_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(file_mapping_handle);
		file_mapping_handle = INVALID_HANDLE_VALUE;
	}

	if (file_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(file_handle);
		file_handle = INVALID_HANDLE_VALUE;
	}
}
