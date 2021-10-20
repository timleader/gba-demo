
#include "savegame.h"

savegame_slots_ptr savegame_slots;


void savegame_initialize(void) 
{
	savegame_slots = (savegame_slots_ptr)0x0E000000;
}

void savegame_shutdown(void)
{ }
