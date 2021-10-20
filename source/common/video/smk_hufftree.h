/**
	libsmacker - A C library for decoding .smacker_handle_ptr Smacker Video files
	Copyright (C) 2012-2017 Greg Kennedy

	See smacker.h for more information.

	smk_hufftree.h
		SMK huffmann trees.  There are two types:
		- a basic 8-bit tree, and
		- a "big" 16-bit tree which includes a cache for recently
			searched values.
*/

#ifndef SMK_HUFFTREE_H
#define SMK_HUFFTREE_H

#include "common/utils/bitstream.h"

/** Tree node structures - Forward declaration */

/**
	8-bit Tree node structure.
	If b0 is non-null, this is a branch, and b1 from the union should be used.
	If b0 is null, this is a leaf, and val / escape code from union should be used.
*/
struct smk_huff8_t		//	using smk_huff8_v5_s here could be useful 
{
	struct smk_huff8_t* b0;		//	offset to ptr
	union
	{
		struct smk_huff8_t* b1;		//	 offset to ptr
		struct
		{
			unsigned short value;
			unsigned char escapecode;
		} leaf;
	} u;
};

/**
	16-bit Tree root struct: holds a huff8_t structure,
	as well as a cache of three 16-bit values.
*/
struct smk_huff16_t		//	this can sit in EWRAM
{
	struct smk_huff8_t* t;		//	this can point to ROM
	unsigned short cache[3];
};

/*********************** 8-BIT HUFF-TREE FUNCTIONS ***********************/

/** Build an 8-bit tree from a bitstream */
struct smk_huff8_t* smk_huff8_build(bitstream_t* bs);

void smk_huff8_count_nodes(const struct smk_huff8_t* t, uint32_t* count);

/** Look up an 8-bit value in the referenced tree by following a bitstream
	returns -1 on error */
short smk_huff8_lookup(bitstream_t* bs, const struct smk_huff8_t* t);

/** function to recursively delete an 8-bit huffman tree */
void smk_huff8_free(struct smk_huff8_t* t);

/************************ 16-BIT HUFF-TREE FUNCTIONS ************************/

/** Build a 16-bit tree from a bitstream */
struct smk_huff16_t* smk_huff16_build(bitstream_t* bs);

/** Look up a 16-bit value in the bigtree by following a bitstream
	returns -1 on error */
long smk_huff16_lookup(bitstream_t* bs, struct smk_huff16_t* big);

/** Reset the cache in a 16-bit tree */
void smk_huff16_reset(struct smk_huff16_t* big);

/** function to recursively delete a 16-bit huffman tree */
void smk_huff16_free(struct smk_huff16_t* big);

#endif
