
#include "lz77.h"

#include "common/memory.h"
#include "common/debug/debug.h"


void lz77_decompress(void_ptr in, const void_ptr out)		
{
	uint8_t* source = in;
	uint8_t* dest = out;

	int32_t remaining = (*((uint32_t*)source) & 0xFFFFFF00) >> 8;

	// We assume the signature byte (0x10) is correct
	int32_t blockheader = 0; // Some compilers warn if this isn't set, even though it's trivially provably always set
	source += 4;

	int32_t blocksRemaining = 0;
	uint8_t* disp;
	int32_t bytes;
	int32_t byte;

	while (remaining > 0) 
	{
		if (blocksRemaining)
		{
			if (blockheader & 0x80)
			{
				// Compressed
				int32_t block = *(source + 1) | (*(source) << 8);
				source += 2;
				disp = dest - (block & 0x0FFF) - 1;
				bytes = (block >> 12) + 3;
				while (bytes--)
				{
					debug_assert(remaining, "lz77:decode, invalid lz77 compressed data");

					--remaining;

					byte = *disp++;
					*dest++ = byte;
				}
			}
			else 
			{
				// Uncompressed
				--remaining;

				byte = *source++;
				*dest++ = byte;
			}
			blockheader <<= 1;
			--blocksRemaining;
		}
		else
		{
			blockheader = *source;
			++source;
			blocksRemaining = 8;
		}
	}

}
