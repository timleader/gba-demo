
#include "bitstream.h"

#include "common/types.h"
#include "common/memory.h"


/* BITSTREAM Functions */
void bitstream_initialize(bitstream_t* bs, const void_ptr ptr, const uint32_t size)
{
	uint32_t addr = (uint32_t)ptr;		
	uint32_t aligned_addr = addr & ~0x03;

	/* set up the pointer to bitstream, and the size counter */
	bs->buffer = (uint32_t*)aligned_addr;// aligned_addr;
	bs->size = size;

	/* point to initial byte: note, smk_malloc already sets these to 0 */
	bs->buffer_idx = 0;
	bs->bit_idx = (addr & 0x03) << 3;

	bs->value = bs->buffer[bs->buffer_idx];
}

IWRAM_CODE uint8_t bitstream_read_1(bitstream_t* bs)	//	should be asm 
{
	uint8_t ret = ~0;

	/* get next bit and return */
	ret = (bs->value >> bs->bit_idx) & 0x01;

	/* advance to next bit */
	bs->bit_idx++;

	/* Out of bits in this byte: next! */
	if (bs->bit_idx > 0x1F)
	{
		bs->buffer_idx++;
		bs->bit_idx = 0;

		bs->value = bs->buffer[bs->buffer_idx];
	}

	return ret;
}

IWRAM_CODE uint8_t bitstream_read_8(bitstream_t* bs)
{
	uint8_t ret = 0;

	if (bs->bit_idx + 8 < 32)
	{
		/* read from value */
		ret = (bs->value >> bs->bit_idx) & 0xFF;
		bs->bit_idx += 8;
	}
	else
	{
		ret = (bs->value >> bs->bit_idx) & 0xFF;

		bs->buffer_idx++;

		bs->bit_idx = (bs->bit_idx + 8) & 0x1F;
		bs->value = bs->buffer[bs->buffer_idx];

		ret |= (bs->value << (8 - bs->bit_idx)) & 0xFF;
	}

	return ret;
}

IWRAM_CODE uint16_t bitstream_read_16(bitstream_t* bs)
{
	uint8_t ret = 0;

	if (bs->bit_idx + 16 < 32)
	{
		/* read from value */
		ret = (bs->value >> bs->bit_idx) & 0xFFFF;
		bs->bit_idx += 16;
	}
	else
	{
		ret = (bs->value >> bs->bit_idx) & 0xFFFF;
		bs->buffer_idx++;

		bs->bit_idx = (bs->bit_idx + 16) & 0x1F;
		bs->value = bs->buffer[bs->buffer_idx];

		ret |= (bs->value << (16 - bs->bit_idx)) & 0xFFFF;
	}

	return ret;
}

IWRAM_CODE uint32_t bitstream_read_32(bitstream_t* bs)
{
	uint32_t ret;

	if (bs->bit_idx == 0)
	{
		ret = bs->value;
		bs->buffer_idx++;
		bs->value = bs->buffer[bs->buffer_idx];
	}
	else
	{
		ret = bs->value >> bs->bit_idx;
		bs->buffer_idx++;

		bs->value = bs->buffer[bs->buffer_idx];

		uint8_t remaining_bits = 32 - bs->bit_idx;
		ret |= bs->value << remaining_bits;
	}

	return ret;
}


IWRAM_CODE uint32_t bitstream_peek_32(bitstream_t* bs)
{
	uint32_t ret;

	if (bs->bit_idx == 0)
	{
		ret = bs->value;
	}
	else
	{
		uint8_t remaining_bits = 32 - bs->bit_idx;
		ret = bs->value >> bs->bit_idx;
		ret |= bs->buffer[bs->buffer_idx + 1] << remaining_bits;
	}

	return ret;
}

IWRAM_CODE void bitstream_move(bitstream_t* bs, uint16_t bit_count)
{
	bit_count += bs->bit_idx;
	bs->buffer_idx += bit_count >> 5;
	bs->bit_idx = bit_count & 0x1F;

	bs->value = bs->buffer[bs->buffer_idx];
}

