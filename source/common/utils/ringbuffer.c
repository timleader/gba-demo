
#include "ringbuffer.h"

#include "common/memory.h"
#include "common/debug/debug.h"


void ringbuffer_new(ringbuffer_ptr buffer, uint16_t initial_capacity, uint8_t memory_sector)
{
	buffer->buffer_size = initial_capacity;
	buffer->data = memory_allocate(initial_capacity, memory_sector);
	buffer->head = buffer->data;
	buffer->tail = buffer->head;
}

void ringbuffer_delete(ringbuffer_ptr buffer)
{
	debug_assert(buffer->data, "ringbuffer:delete was called on an uninitialized ringbuffer");

	memory_free(buffer->data);

	buffer->data = NULL;
}

void ringbuffer_write(ringbuffer_ptr buffer, int8_t value)	// inline
{
	*buffer->tail++ = value;

	if (buffer->tail >= buffer->data + buffer->buffer_size)
		buffer->tail = buffer->data;

	debug_assert(buffer->head != buffer->tail, "ringbuffer:write overflow");		//	assert head and tail don't cross
}

void ringbuffer_copy_from(ringbuffer_ptr buffer, int8_t* dest, uint16_t size)	// pull
{
	//	2 passes	speed this up 
	
	while (size > 0)
	{
		*dest++ = *buffer->head++;

		debug_assert(buffer->head != buffer->tail, "ringbuffer:write copy_from");		//	assert head and tail don't cross

		if (buffer->head >= buffer->data + buffer->buffer_size)
			buffer->head = buffer->data;

		size--;
	}
}

uint32_t ringbuffer_used(ringbuffer_ptr buffer)
{
	uint32_t used = 0;

	if (buffer->head == buffer->tail)
		used = 0;
	else if (buffer->head > buffer->tail)
		used = buffer->buffer_size - (buffer->head - buffer->tail);
	else
		used = buffer->tail - buffer->head;

	return used;
}

uint32_t ringbuffer_unused(ringbuffer_ptr buffer)
{
	return buffer->buffer_size - ringbuffer_used(buffer);
}
