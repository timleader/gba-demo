
#include "savegame.h"

#include "common/types.h"
#include "common/debug/debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>


HANDLE savegame_file_handle;
HANDLE savegame_file_mapping_handle;
uint8_t* raw_savegame_data_ptr;

savegame_slots_ptr savegame_slots;


void savegame_initialize(void) 
{
	savegame_file_handle = CreateFile(
		"../data/7days.sav", GENERIC_READ | GENERIC_WRITE, 0, 0,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	if (savegame_file_handle == INVALID_HANDLE_VALUE)	///	this is a fatal error
		return;

	DWORD high_size = 0;
	DWORD low_size = SAVEGAME_SLOT_SIZE * SAVEGAME_SLOT_COUNT;

	size_t mapping_size = high_size;
	mapping_size <<= 32;
	mapping_size |= low_size;

	savegame_file_mapping_handle = CreateFileMapping(
		savegame_file_handle, 0, PAGE_READWRITE, high_size, low_size, 0);

	if (savegame_file_mapping_handle == INVALID_HANDLE_VALUE)
		return;

	raw_savegame_data_ptr = (uint8_t*)(MapViewOfFile(
		savegame_file_mapping_handle, FILE_MAP_WRITE, 0, 0, mapping_size));

	if (!raw_savegame_data_ptr)
		return;


	savegame_slots = (savegame_slots_ptr)raw_savegame_data_ptr;
}

void savegame_shutdown(void)
{
	if (raw_savegame_data_ptr)
	{
		UnmapViewOfFile(raw_savegame_data_ptr);
		raw_savegame_data_ptr = NULL;
	}

	if (savegame_file_mapping_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(savegame_file_mapping_handle);
		savegame_file_mapping_handle = INVALID_HANDLE_VALUE;
	}

	if (savegame_file_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(savegame_file_handle);
		savegame_file_handle = INVALID_HANDLE_VALUE;
	}
}
