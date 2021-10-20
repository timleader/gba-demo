/**
	libsmacker - A C library for decoding .smacker_handle_ptr Smacker Video files
	Copyright (C) 2012-2017 Greg Kennedy

	See smacker.h for more information.

	smk_hufftree.c
		Implementation of Smacker Huffman coding trees.
*/

#include "smk_hufftree_v5.h"		//	this might turn out to be stupid ... 

#include "common/platform/gba/gba.h"
#include "common/memory.h"
#include "common/debug/debug.h"
#include "common/containers/list.h"


IWRAM_CODE smk_huff8_v5_t* smk_huff8_v5_build(bitstream_t* bs)
{
	const int32_t max_tree_size = 96;

	//	preallocate 
	smk_huff8_v5_t* head = (smk_huff8_v5_t*)memory_allocate(sizeof(smk_huff8_v5_t) * max_tree_size, MEMORY_IWRAM);
	smk_huff8_v5_t* node = head;
	smk_huff8_v5_t* tail = head;
	bool8_t bit;

	int16_list_t b1_parent_offsets;
	list_new(&b1_parent_offsets, 32, MEMORY_IWRAM);

	debug_assert(bs, "smk_huff8_build: bs is NULL");

	/* Smacker huff trees begin with a set-bit. */
	bit = bitstream_read_1(bs);
	debug_assert(bit, "Got a bit, but it was not 1. In theory, there could be a smacker_handle_ptr file");

	for (;;)
	{
		/* Read the bit - is_branch  */
		bit = bitstream_read_1(bs);
		if (bit)	//	branch
		{
			node->branch.branch_1_offset = 0xffff;
			node->branch.is_branch_node = 0x0f;

			list_append(&b1_parent_offsets, node - head);

			node += 1;
			if (node > tail)
				tail = node;

			debug_assert(node < head + max_tree_size, "smk_huff8_v5_build: overflowed tree size");
		}
		else	//	leaf
		{
			node->leaf.value = bitstream_read_8(bs);
			node->leaf.escapecode = 0xff;

			while (b1_parent_offsets.length > 0)
			{
				//	move back up to parent 
				int16_t offset = b1_parent_offsets.data[b1_parent_offsets.length - 1];
				node = head + offset;	

				//	if b1 is not done move down b1 
				if (node->branch.branch_1_offset != 0xffff)
				{
					list_remove_at_index(&b1_parent_offsets, b1_parent_offsets.length - 1);
				}
				else
				{
					++tail;
					node->branch.branch_1_offset = tail - node;
					node = tail;

					debug_assert(node < head + max_tree_size, "smk_huff8_v5_build: overflowed tree size");
					break;
				}
			}

			if (b1_parent_offsets.length == 0)
				break;
		}
	}

	/* huff trees end with an unset-bit */
	bit = bitstream_read_1(bs);
	debug_assert(!bit, "ERROR: final get_bit returned 1");

	list_delete(&b1_parent_offsets);

	return head;
}


/* Look up an 8-bit value from a basic huff tree. Return -1 on error. */
IWRAM_CODE int16_t smk_huff8_v5_lookup(bitstream_t* bs, const smk_huff8_v5_t* t)
{
	bool8_t bit;
	smk_huff8_v5_t* tp = (smk_huff8_v5_t *)t;

	while (tp->branch.is_branch_node == 0x0F)
	{
		/* Read the next bit from bitstream to determine path */
		bit = bitstream_read_1(bs);

		if (bit)
		{
			/* get_bit returned Set, follow Right branch. */
			tp += tp->branch.branch_1_offset;			//	loop rather than function call ??? 
		}
		else
		{
			tp += 1;
		}
	}

	/* Reached a Leaf node. Return its value. */
	return tp->leaf.value;
}

/*********************** 16-BIT HUFF-TREE FUNCTIONS ***********************/

/* Convenience call-out for recursive bigtree lookup function */
IWRAM_CODE uint16_t smk_huff16_v5_lookup(bitstream_t* bs, smk_huff16_v5_t* big)		// remove this function step
{
	uint16_t val;
	char bit;
	smk_huff8_v5_t* tp = big->t;
	uint16_t* cache = big->cache;


	uint32_t bs_value = bitstream_peek_32(bs);		//	validate that tree is no deeper than 32 ... 
	uint8_t bit_count = 0;

	// how many times is the same tree path repeated, could cache last path and vale  

	while (tp->branch.is_branch_node == 0x0F)		//	average of 7 loops		//	how many leaf nodes  
	{
		/* Read the next bit from bitstream to determine path */
		bit = (bs_value & 0x01) > 0;		//	this should be inline !!!		//	pull a int16/int8 at a time
		bs_value >>= 1;	//	this can only be done 32 times ... 

		if (bit)
		{
			tp += tp->branch.branch_1_offset;		
		}
		else
		{
			tp += 1;
		}

		bit_count++;
	}

	bitstream_move(bs, bit_count);

	/* Reached a Leaf node */
	if (tp->leaf.escapecode != 0xFF)
	{
		/* Found escape code. Retrieve value from Cache. */
		val = cache[tp->leaf.escapecode];
	}
	else
	{
		/* Use value directly. */
		val = tp->leaf.value;
	}

	if (cache[0] != val)
	{
		/* Update the cache, by moving val to the front of the queue,
			if it isn't already there. */
		cache[2] = cache[1];
		cache[1] = cache[0];
		cache[0] = val;
	}

	return val;
}

/* Resets a Big hufftree cache */
void smk_huff16_v5_reset(smk_huff16_v5_t* big)
{
	big->cache[0] = 0;
	big->cache[1] = 0;
	big->cache[2] = 0;
}

