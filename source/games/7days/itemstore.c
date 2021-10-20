
#include "itemstore.h"

#include "common/memory.h"
#include "common/resources/resources.h"

itemstore_ptr itemstore;

void itemstore_initialize(void)
{
	itemstore = resources_find_itemstore_from_name("item/store");

	debug_assert(itemstore != NULL, "itemstore::initialize itemstore == NULL");
}

item_ptr itemstore_get(uint16_t id)
{
	debug_assert(id < itemstore->count, "itemstore::get id >= itemstore->count");

	return &itemstore->items[id];
}
