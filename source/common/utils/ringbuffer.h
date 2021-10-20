
#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include "common/types.h"


typedef struct int8_ringbuffer_s
{
	int8_t* data;
	int8_t* head, * tail;

	int32_t buffer_size;

} ringbuffer_t;

typedef ringbuffer_t* ringbuffer_ptr;


void ringbuffer_new(ringbuffer_ptr buffer, uint16_t initial_capacity, uint8_t memory_sector);

void ringbuffer_delete(ringbuffer_ptr buffer);

void ringbuffer_write(ringbuffer_ptr buffer, int8_t value);

void ringbuffer_copy_from(ringbuffer_ptr buffer, int8_t* dest, uint16_t size);	// pull / read


uint32_t ringbuffer_used(ringbuffer_ptr buffer);

uint32_t ringbuffer_unused(ringbuffer_ptr buffer);


#endif

