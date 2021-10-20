
#include "lz77.h"

#include "common/memory.h"
#include "common/debug/debug.h"


//-----------------------------------------------------------------------------
void lz77_decompress_vram(void_ptr in, const void_ptr out);
void lz77_decompress_wram(void_ptr in, const void_ptr out);


//-----------------------------------------------------------------------------
void lz77_decompress(void_ptr in, const void_ptr out)
{
	if (out < 0x06000000)
	{
		lz77_decompress_wram(in, out);
	}
	else
	{
		lz77_decompress_vram(in, out);
	}
}
