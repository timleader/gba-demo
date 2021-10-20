/**
	libsmacker - A C library for decoding .smacker_handle_ptr Smacker Video files
	Copyright (C) 2012-2017 Greg Kennedy

	See smacker.h for more information.

	smk_hufftree.c
		Implementation of Smacker Huffman coding trees.
*/

#include "smk_hufftree.h"

#include "common/types.h"
#include "common/memory.h"
#include "common/debug/debug.h"

/*********************** 8-BIT HUFF-TREE FUNCTIONS ***********************/

/** Recursive tree-building function. */
static struct smk_huff8_t* smk_huff8_build_rec(bitstream_t* bs)		//	optimize this ... 
{
	struct smk_huff8_t* ret = NULL;
	char bit;

	/* sanity check - removed: bs cannot be null, because it was checked at smk_huff8_build below */
	/* smk_assert(bs); */

	/* Read the bit */
	bit = bitstream_read_1(bs);

	/* Malloc a structure. */
	ret = memory_allocate(sizeof(struct smk_huff8_t), MEMORY_DEVELOPMENT);		//	this won't be GBA compatible 
	memory_set(ret, sizeof(struct smk_huff8_t), 0);

	/*
		maybe pre-allocate a pool, then bump alloc from that ..
	*/

	if (bit)
	{
		/* Bit set: this forms a Branch node. */
		/* Recursively attempt to build the Left branch. */
		ret->b0 = smk_huff8_build_rec(bs);			//	stop this from being recursive

		/* Everything is still OK: attempt to build the Right branch. */
		ret->u.b1 = smk_huff8_build_rec(bs);

		/* return branch pointer here */
		return ret;
	}

	/* Bit unset signifies a Leaf node. */
	/* Attempt to read value */
	ret->u.leaf.value = bitstream_read_8(bs);

	/* smk_malloc sets entries to 0 by default */
	/* ret->b0 = NULL; */
	ret->u.leaf.escapecode = 0xFF;

	return ret;
}

void smk_huff8_count_nodes(const struct smk_huff8_t* t, uint32_t* count)
{
	if (t->b0)
	{
		smk_huff8_count_nodes(t->b0, count);
		smk_huff8_count_nodes(t->u.b1, count);
	}

	*count += 1;
}

/* Look up an 8-bit value from a basic huff tree.
	Return -1 on error. */
short smk_huff8_lookup(bitstream_t* bs, const struct smk_huff8_t* t)		//	non-recursive implementation please ... 
{
	char bit;

	debug_assert(t, "smk_hufftree is NULL");

	if (!t->b0)
	{
		/* Reached a Leaf node. Return its value. */
		return t->u.leaf.value;
	}

	/* Read the next bit from bitstream to determine path */
	bit = bitstream_read_1(bs);

	if (bit)
	{
		/* get_bit returned Set, follow Right branch. */
		return smk_huff8_lookup(bs, t->u.b1);
	}

	/* follow Left branch */
	return smk_huff8_lookup(bs, t->b0);
}

/**
	Entry point for huff8 build. Basically just checks the start/end tags
	and calls smk_huff8_build_rec recursive function.
*/
struct smk_huff8_t* smk_huff8_build(bitstream_t* bs)		
{
	struct smk_huff8_t* ret = NULL;
	char bit;

	debug_assert(bs, "smk_huff8_build: bs is NULL"); 

	/* Smacker huff trees begin with a set-bit. */
	bit = bitstream_read_1(bs);

	debug_assert(bit, "Got a bit, but it was not 1. In theory, there could be a smacker_handle_ptr file");

	/* Begin parsing the tree data. */
	ret = smk_huff8_build_rec(bs);

	/* huff trees end with an unset-bit */
	bit = bitstream_read_1(bs);

	debug_assert(!bit, "ERROR: final get_bit returned 1");

	return ret;
}

/* function to recursively delete a huffman tree */
void smk_huff8_free(struct smk_huff8_t* t)
{
	/* Sanity check: do not double-free 
	smk_assert(t);
	*/

	/* If this is not a leaf node, free child trees first */
	if (t->b0)
	{
		smk_huff8_free(t->b0);
		smk_huff8_free(t->u.b1);
	}

	/* Safe-delete tree node. */
	memory_free(t);
}

/*********************** 16-BIT HUFF-TREE FUNCTIONS ***********************/

/* Recursively builds a Big tree. */
static struct smk_huff8_t* smk_huff16_build_rec(bitstream_t* bs, const unsigned short cache[3], const struct smk_huff8_t* low8, const struct smk_huff8_t* hi8)
{
	struct smk_huff8_t* ret = NULL;

	char bit;
	short lowval;

	/* sanity check - removed: these cannot be null, because they were checked at smk_huff16_build below */
	/* smk_assert(bs);
	smk_assert(cache);
	smk_assert(low8);
	smk_assert(hi8); */

	/* Get the first bit */
	bit = bitstream_read_1(bs);

	/* Malloc a structure. */
	ret = memory_allocate(sizeof(struct smk_huff8_t), MEMORY_DEVELOPMENT);

	if (bit)
	{
		/* Recursively attempt to build the Left branch. */
		ret->b0 = smk_huff16_build_rec(bs, cache, low8, hi8);

		/* Recursively attempt to build the Left branch. */
		ret->u.b1 = smk_huff16_build_rec(bs, cache, low8, hi8);

		/* return branch pointer here */
		return ret;
	}

	/* Bit unset signifies a Leaf node. */
	lowval = smk_huff8_lookup(bs, low8);
	ret->u.leaf.value = smk_huff8_lookup(bs, hi8);

	/* Looks OK: we got low and hi values. Return a new LEAF */
	/* ret->b0 = NULL; */
	ret->u.leaf.value = lowval | (ret->u.leaf.value << 8);

	/* Last: when building the tree, some Values may correspond to cache positions.
		Identify these values and set the Escape code byte accordingly. */
	if (ret->u.leaf.value == cache[0])
	{
		ret->u.leaf.escapecode = 0;
	}
	else if (ret->u.leaf.value == cache[1])
	{
		ret->u.leaf.escapecode = 1;
	}
	else if (ret->u.leaf.value == cache[2])
	{
		ret->u.leaf.escapecode = 2;
	}
	else
	{
		ret->u.leaf.escapecode = 0xFF;
	}

	return ret;
}

/* Entry point for building a big 16-bit tree. */
struct smk_huff16_t* smk_huff16_build(bitstream_t* bs)
{
	struct smk_huff16_t* big = NULL;

	struct smk_huff8_t* low8 = NULL;
	struct smk_huff8_t* hi8 = NULL;

	uint8_t lowval, highval;

	char bit;
	unsigned char i;

	/* sanity check 
	smk_assert(bs);*/

	/* Smacker huff trees begin with a set-bit. */
	bit = bitstream_read_1(bs);

	if (!bit)
	{
		//fputs("libsmacker::smk_huff16_build(bs) - ERROR: initial get_bit returned 0\n", stderr);
		goto error;
	}

	/* build low-8-bits tree */
	low8 = smk_huff8_build(bs);		//	this fails 
	/* build hi-8-bits tree */
	hi8 = smk_huff8_build(bs);

	/* Everything looks OK so far. Time to malloc structure. */
	big = memory_allocate(sizeof(struct smk_huff16_t), MEMORY_DEVELOPMENT);
	if (big == NULL)
	{
		big = NULL;
	}

	/* Init the escape code cache. */
	for (i = 0; i < 3; i ++)
	{
		lowval = bitstream_read_8(bs);
		highval = bitstream_read_8(bs);

		big->cache[i] = lowval | (highval << 8);
	}

	/* Finally, call recursive function to retrieve the Bigtree. */
	big->t = smk_huff16_build_rec(bs, big->cache, low8, hi8);

	/* Done with 8-bit hufftrees, free them. */
	smk_huff8_free(hi8);
	smk_huff8_free(low8);

	/* Check final end tag. */
	bit = bitstream_read_1(bs);

	if (bit)
	{
		//fputs("libsmacker::smk_huff16_build(bs) - ERROR: final get_bit returned 1\n", stderr);
		goto error;
	}

	return big;

error:
	smk_huff16_free(big);
	smk_huff8_free(hi8);
	smk_huff8_free(low8);
	return NULL;
}

static int smk_huff16_lookup_rec(bitstream_t* bs, unsigned short cache[3], const struct smk_huff8_t* t)
{
	unsigned short val;
	char bit; 
	struct smk_huff8_t* tp = (struct smk_huff8_t*)t;

	/* sanity check */
	/* smk_assert(bs);
	smk_assert(cache);
	smk_assert(t); */

	while (tp->b0)
	{
		/* Read the next bit from bitstream to determine path */
		bit = bitstream_read_1(bs);

		if (bit)
		{
			/* get_bit returned Set, follow Right branch. */
			tp = tp->u.b1;			//	loop rather than function call ??? 
		}
		else
		{
			tp = tp->b0;
		}
	}

	/* Reached a Leaf node */
	if (tp->u.leaf.escapecode != 0xFF)
	{
		/* Found escape code. Retrieve value from Cache. */
		val = cache[tp->u.leaf.escapecode];
	}
	else
	{
		/* Use value directly. */
		val = tp->u.leaf.value;
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

/* Convenience call-out for recursive bigtree lookup function */
long smk_huff16_lookup(bitstream_t* bs, struct smk_huff16_t* big)		// remove this function step
{
	return smk_huff16_lookup_rec(bs, big->cache, big->t);
}

/* Resets a Big hufftree cache */
void smk_huff16_reset(struct smk_huff16_t* big)
{
	big->cache[0] = 0;
	big->cache[1] = 0;
	big->cache[2] = 0;
}

/* delete a (big) huffman tree */
void smk_huff16_free(struct smk_huff16_t* big)
{
	/* free the subtree */
	if (big->t)
		smk_huff8_free(big->t);

	/* free the bigtree */
	memory_free(big);

}
