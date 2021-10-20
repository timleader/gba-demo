
#ifndef BITSTREAM_H
#define BITSTREAM_H

#include "common/platform/gba/gba.h"

/*
	Should explicitly be read_bitstream
*/

/*
	Bitstream structure
	Pointer to raw block of data and a size limit.
	Maintains internal pointers to byte_num and bit_number.
*/
typedef struct bitstream_s	
{
	const uint32_t* buffer;
	uint32_t size;

	uint32_t buffer_idx;

	uint8_t bit_idx;
	uint8_t reserved[3];

	uint32_t value;	//	cache value

} bitstream_t;

/* BITSTREAM Functions */
/** Initialize a bitstream */
void bitstream_initialize(bitstream_t* bs, const void_ptr ptr, const uint32_t size);


IWRAM_CODE uint8_t bitstream_read_1(bitstream_t* bs);

IWRAM_CODE uint8_t bitstream_read_8(bitstream_t* bs);

IWRAM_CODE uint16_t bitstream_read_16(bitstream_t* bs);

IWRAM_CODE uint32_t bitstream_read_32(bitstream_t* bs);


IWRAM_CODE uint32_t bitstream_peek_32(bitstream_t* bs);

IWRAM_CODE void bitstream_move(bitstream_t* bs, uint16_t bit_count);

#endif
