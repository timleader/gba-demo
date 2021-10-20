
#ifndef INVENTORY_H
#define INVENTORY_H

#include "common/types.h"

#define INVENTORY_MAX_COUNT 12

typedef struct inventory_s
{
	int16_t items[INVENTORY_MAX_COUNT];

} inventory_t;

typedef inventory_t* inventory_ptr;


void inventory_reset(inventory_ptr inventory);

uint8_t inventory_count(inventory_ptr inventory);

void inventory_additem(inventory_ptr inventory, uint16_t item_id);

uint8_t inventory_hasitem(inventory_ptr inventory, uint16_t item_id);

void inventory_removeitem(inventory_ptr inventory, uint16_t item_id);


#endif
