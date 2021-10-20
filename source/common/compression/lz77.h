
#ifndef LZ77_H
#define LZ77_H

#include "common/utils/bitstream.h"


void lz77_decompress(void_ptr in, const void_ptr out);		//	uses gba bios function 


//	0x13	HuffUnComp
//	0x10	BitUnPack
//	0x14	RLUnCompWRAM
//	0x16	Diff8bitUnFilterWRAM


#endif
