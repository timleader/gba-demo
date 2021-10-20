
#ifndef ITEMSTORE_H
#define ITEMSTORE_H

#include "common/types.h"

typedef struct item_s
{
	uint16_t active_image_id;
	uint16_t inactive_image_id;

	//	large image		??

	uint16_t name_text_id;
	uint16_t reserved;

	//	description text	??

} item_t;

typedef item_t* item_ptr;

typedef struct itemstore_s
{
	uint8_t count;
	uint8_t reserved[3];

	item_t items[0];

} itemstore_t;

typedef itemstore_t* itemstore_ptr;


void itemstore_initialize(void);

item_ptr itemstore_get(uint16_t id);


#endif
