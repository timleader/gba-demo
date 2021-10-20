
#ifndef MEMORY_H
#define MEMORY_H

/*
	might as well all be 16-bit aligned, 
		optionally request 32-bit alignment.

	memory budgeting 
	memory usage output to log
*/

#include "types.h"

//-----------------------------------------------------------------------------
#define MEMORY_IWRAM 0
#define MEMORY_EWRAM 1
#define MEMORY_DEVELOPMENT	2		//	maybe bypass instead - to go straight to malloc 

//-----------------------------------------------------------------------------
typedef struct memory_section_s
{
	void_ptr start;
	uint32_t size;

	uint32_t used;

} memory_section_t;


//-----------------------------------------------------------------------------
void memory_initialize(void);

//	All allocate calls should result in a 4 byte aligned block of memory
//-----------------------------------------------------------------------------
void_ptr memory_allocate(uint32_t size_in_bytes, uint8_t sector);	//	labels only for debug ... 

//-----------------------------------------------------------------------------
void memory_free(void_ptr ptr);

//void memory_reset();

//-----------------------------------------------------------------------------
void memory_copy(void_ptr dest, const void_ptr src, const uint32_t byte_count);
void memory_copy8(void_ptr dest, const void_ptr src, const uint32_t byte_count);
void memory_copy16(void_ptr dest, const void_ptr src, const uint32_t half_word_count);
void memory_copy32(void_ptr dest, const void_ptr src, const uint32_t word_count);

//-----------------------------------------------------------------------------
void memory_dma_copy32(void_ptr dest, const void_ptr src, const uint32_t word_count);

//-----------------------------------------------------------------------------
void memory_set(void_ptr dest, const uint32_t byte_count, const uint8_t value);
void memory_set32(void_ptr dest, const uint32_t word_count, const uint32_t value);

/*
	Calls: allocation : number of calls
	EWRAM: Used / Size
	IWRAM: Used / Size 
*/

//-----------------------------------------------------------------------------
uint32_t memory_usage(uint8_t sector);
void memory_output_usage(void);
//	output memory to csv 


#endif 
