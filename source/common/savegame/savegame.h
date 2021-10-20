
#ifndef SAVEGAME_H
#define SAVEGAME_H

#include "common/types.h"

/*
	//	this can move to common 

	Game Pak SRAM - 64 KBytes

	save slots ??? 

		//	how much do we need ???

		1K per save 


		//	more of a saving process than structure !!! 

*/

//	32 kB SRAM -- 3 slots -> 48 kB 


#define SAVEGAME_SIGNATURE		0xAA

#define SAVEGAME_STATE_OPEN		0x01
#define SAVEGAME_STATE_CLOSE	0x00

#define SAVEGAME_VERSION		0x01

#define SAVEGAME_NAME_SIZE		20

#define SAVEGAME_SLOT_HEADER_SIZE	24
#define SAVEGAME_SLOT_SIZE		16384	

#define SAVEGAME_SLOT_COUNT		2


typedef struct savegame_s
{
	uint8_t signature;		//	if signature isn't set 
	uint8_t state;
	uint8_t version; 
	uint8_t reserved;

	char name[SAVEGAME_NAME_SIZE];

	uint8_t data[SAVEGAME_SLOT_SIZE - SAVEGAME_SLOT_HEADER_SIZE];		//	currently needs 10292

	//	image block

} savegame_t;

typedef savegame_t* savegame_ptr;

typedef struct savegame_slots_s		//	point this to	0x0E000000
{
	savegame_t slots[SAVEGAME_SLOT_COUNT];

} savegame_slots_t;

typedef savegame_slots_t* savegame_slots_ptr;


extern savegame_slots_ptr savegame_slots;


void savegame_initialize(void);

void savegame_shutdown(void);


void savegame_write_header(savegame_ptr slot, const char* name);

void savegame_set_open(savegame_ptr slot);

void savegame_set_close(savegame_ptr slot);

uint8_t savegame_has_savedata(savegame_ptr slot);

//	function to set writing, complete writing 

//	function to list avalible save slots 	

//	

#endif

