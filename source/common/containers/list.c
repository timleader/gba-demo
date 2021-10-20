
#include "list.h"

#include "common/memory.h"
#include "common/debug/debug.h"


void list_new(int16_list_ptr list, uint16_t initial_capacity, uint8_t memory_sector)
{
	list->length = 0;
	list->capacity = initial_capacity;
	list->data = memory_allocate(initial_capacity * sizeof(int16_t), memory_sector);
}

void list_delete(int16_list_ptr list)
{
	debug_assert(list->data, "list:delete was called on an uninitialized list");

	memory_free(list->data);

	list->length = 0;
	list->capacity = 0;
	list->data = NULL;
}

int16_t list_append(int16_list_ptr list, int16_t value)
{
	if (list->capacity == list->length)
	{
		list_resize(list, list->capacity + 8);
	}

	list->data[list->length] = value;
	list->length++;

	return 0;
}

int16_t list_resize(int16_list_ptr list, uint16_t new_size)
{
	int16_t* oldptr = list->data;
	int16_t* newptr = memory_allocate(new_size * sizeof(int16_t), MEMORY_EWRAM);

	for (int16_t i = 0; i < list->length; ++i)
	{
		newptr[i] = oldptr[i];
	}

	memory_free(oldptr);

	list->data = newptr;

	return 1;
}

int16_t list_insert(int16_list_ptr list, int16_t value, uint16_t index)
{
	/*	just fail istead, lets not waste cycles !!!

	if (idx < 0)
	{
		idx = 0;
	}
	else if (idx > list->element_count)
	{
		idx = list->element_count;
	}
	*/

	for (uint16_t i = list->length; i > index; --i)
	{
		list->data[i] = list->data[i - 1];
	}

	/* set new obj at insert index */
	list->data[index] = value;
	list->length++;

	return index;
}

void list_remove_at_index(int16_list_ptr list, uint16_t index)
{

	for (uint16_t i = list->length - 1; i > index; --i)
	{
		list->data[i - 1] = list->data[i];
	}

	list->length--;
}

int16_t list_index_of(int16_list_ptr list, int16_t value)
{
	int16_t index = -1;
	for (int16_t i = 0; i < list->length; ++i)
	{
		if (list->data[i] == value)
		{
			index = i;
			break;
		}
	}
	return index;
}

void list_reverse(int16_list_ptr list)
{
	int16_t head_idx = 0;
	int16_t tail_idx = list->length - 1;

	while (head_idx < tail_idx)
	{
		int16_t temp_value = list->data[tail_idx];
		list->data[tail_idx] = list->data[head_idx];
		list->data[head_idx] = temp_value;

		head_idx++;
		tail_idx--;
	}
}

