/**
	libsmacker - A C library for decoding .smacker_handle_ptr Smacker Video files
	Copyright (C) 2012-2017 Greg Kennedy

	See smacker.h for more information.

	smk_hufftree_rom.h
		SMK huffmann trees.  There are two types:
		- a basic 8-bit tree, and
		- a "big" 16-bit tree which includes a cache for recently
			searched values.
*/

#ifndef SMK_HUFFTREE_V5_H
#define SMK_HUFFTREE_V5_H

#include "smk_hufftree.h"

#include "common/types.h"
#include "common/utils/bitstream.h"

/** Tree node structures - Forward declaration */
/**
	8-bit Tree node structure.
	If b0 is non-null, this is a branch, and b1 from the union should be used.
	If b0 is null, this is a leaf, and val / escape code from union should be used.
*/

typedef union smk_huff8_v5_s			//	breath-free storage of these structs, then we might be able to use uint8_t for the b0, b1
{
	struct
	{
		uint16_t branch_1_offset;		//	 offset to branch 1		// does this ever, realistically 
		uint8_t is_branch_node;
		uint8_t reserved;
	} branch;
	struct
	{
		uint16_t value;					//	8-bit audio only uses 8-bits here however video always uses the full 16-bits here 
		uint8_t escapecode;
		uint8_t reserved;
	} leaf;

} smk_huff8_v5_t;


/**
	16-bit Tree root struct: holds a huff8_t structure,
	as well as a cache of three 16-bit values.
*/
typedef struct smk_huff16_v5_s		//	this can sit in EWRAM
{
	smk_huff8_v5_t* t;		//	this can point to ROM
	uint16_t cache[3];

} smk_huff16_v5_t;


smk_huff8_v5_t* smk_huff8_v5_build(bitstream_t* bs);

/*********************** 8-BIT HUFF-TREE FUNCTIONS ***********************/

/** Look up an 8-bit value in the referenced tree by following a bitstream
	returns -1 on error */
int16_t smk_huff8_v5_lookup(bitstream_t* bs, const smk_huff8_v5_t* t);

/************************ 16-BIT HUFF-TREE FUNCTIONS ************************/

/** Look up a 16-bit value in the bigtree by following a bitstream
	returns -1 on error */
uint16_t smk_huff16_v5_lookup(bitstream_t* bs, smk_huff16_v5_t* big);

/** Reset the cache in a 16-bit tree */
void smk_huff16_v5_reset(smk_huff16_v5_t* big);

#endif
