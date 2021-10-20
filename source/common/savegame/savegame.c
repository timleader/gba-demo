
#include "savegame.h"

#include "common/memory.h"
#include "common/debug/debug.h"


void savegame_set_open(savegame_ptr slot)
{
	slot->state = SAVEGAME_STATE_OPEN;
}

void savegame_set_close(savegame_ptr slot)
{
	slot->state = SAVEGAME_STATE_CLOSE;
}

void savegame_write_header(savegame_ptr slot, const char* name)
{
	//debug_assert(slot->signature == SAVEGAME_SIGNATURE && slot->state != SAVEGAME_STATE_OPEN);

	slot->signature = SAVEGAME_SIGNATURE;		
	slot->version = SAVEGAME_VERSION;

	memory_copy8(slot->name, (const void_ptr)name, SAVEGAME_NAME_SIZE);
}

uint8_t savegame_has_savedata(savegame_ptr slot)
{
	return slot->signature == SAVEGAME_SIGNATURE;
}


/*
	
	Is Avaliable 




	Open Transaction

	WriteHeader

	WriteData 

	Commit Transaction


*/
