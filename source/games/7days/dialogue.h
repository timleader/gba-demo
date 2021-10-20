
#ifndef DIALOGUE_H
#define DIALOGUE_H

#include "common/math/matrix.h"


typedef struct dialogue_entry_s
{
	uint16_t attributes;
	uint16_t character_image;

	struct
	{
		uint16_t text;
		int16_t next;			//	int16_t so we can support -1 for termination
								//	next can be next diag id or next sequence id, how do/
	}
	options[1];			//	can we support 6 options ?? 

} dialogue_entry_t;

typedef dialogue_entry_t* dialogue_entry_ptr;

typedef struct dialogue_s
{
	uint16_t entry_count;
	uint16_t reserved;

	//	be careful indexing into this, as entry could have multiple options
	dialogue_entry_t entries[0];		

} dialogue_t;

typedef dialogue_t* dialogue_ptr;

#define DIALOGUE_ENTRY_SIZE(N) (sizeof(dialogue_entry_t) + (N * sizeof(uint16_t) * 2))

//	Dialogue_attributes 
//
//	0|1			=>	options_count
//	2|3			=>	layout
//	4|5|6|7		=>	delivery options
//

#endif
