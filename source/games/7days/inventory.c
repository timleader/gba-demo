
#include "inventory.h"

#include "common/debug/debug.h"


void inventory_reset(inventory_ptr inventory)
{
	int8_t idx = 0;
	for (; idx < INVENTORY_MAX_COUNT; ++idx)
	{
		inventory->items[idx] = -1;
	}
}

uint8_t inventory_count(inventory_ptr inventory)
{
	int8_t count = 0;
	for (; count < INVENTORY_MAX_COUNT; ++count)
	{
		if (inventory->items[count] == -1)
			break;
	}
	return count;
}

void inventory_additem(inventory_ptr inventory, uint16_t item_id)
{
	int8_t idx = 0;
	for (; idx < INVENTORY_MAX_COUNT; ++idx)
	{
		if (inventory->items[idx] == -1)
		{
			inventory->items[idx] = item_id;
			debug_printf(DEBUG_LOG_ERROR, "inventory::additem %u", item_id);
			break;
		}
	}

	if (idx == INVENTORY_MAX_COUNT)
		debug_printf(DEBUG_LOG_ERROR, "inventory::additem failed - insufficient space");
}

uint8_t inventory_hasitem(inventory_ptr inventory, uint16_t item_id)
{
	int8_t idx = 0;
	for (; idx < INVENTORY_MAX_COUNT; ++idx)
	{
		if (inventory->items[idx] == item_id)
			break;
	}

	return idx != INVENTORY_MAX_COUNT;
}

void _inventory_compact(inventory_ptr inventory)
{
	int8_t idx = 0;
	int8_t first_free_idx = -1;

	for (; idx < INVENTORY_MAX_COUNT; ++idx)
	{
		if (first_free_idx == -1)
		{
			if (inventory->items[idx] == -1)
			{
				first_free_idx = idx;
			}
		}
		else
		{
			if (inventory->items[idx] >= 0)
			{
				inventory->items[first_free_idx] = inventory->items[idx];
				inventory->items[idx] = -1;

				idx = first_free_idx + 1;
				first_free_idx = -1;
			}
		}
	}
}

void inventory_removeitem(inventory_ptr inventory, uint16_t item_id)
{
	int8_t idx = 0;
	for (; idx < INVENTORY_MAX_COUNT; ++idx)
	{
		if (inventory->items[idx] == item_id)
		{
			inventory->items[idx] = -1;
			debug_printf(DEBUG_LOG_ERROR, "inventory::removeitem %u", item_id);
			break;
		}
	}

	_inventory_compact(inventory);
}
