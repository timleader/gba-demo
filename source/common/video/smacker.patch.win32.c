

#include "smacker.h"

#include "common/memory.h"
#include "common/debug/debug.h"
#include "common/utils/bitstream.h"

/**
	Entry point for huff8 build. Basically just checks the start/end tags
	and calls smk_huff8_build_rec recursive function.
*/
smk_huff8_v5_t* smk_huff8_v5_convert(struct smk_huff8_t* t, uint8_t* output)
{
	smk_huff8_v5_t* p = (smk_huff8_v5_t*)output;

	p->branch.reserved = 0x05;

	if (!t->b0)
	{
		p->leaf.value = t->u.leaf.value;
		p->leaf.escapecode = t->u.leaf.escapecode;

		return p;
	}

	output += sizeof(smk_huff8_v5_t);

	p->branch.is_branch_node = 0xF;
	output = smk_huff8_v5_convert(t->b0, output);

	output += sizeof(smk_huff8_v5_t);

	uint32_t i = (uint32_t)(output - (uint8_t*)p);
	i /= sizeof(smk_huff8_v5_t);
	if (i > 0xFFFF)
	{
		i = 0;
	}

	p->branch.branch_1_offset = (uint16_t)i;
	output = smk_huff8_v5_convert(t->u.b1, output);

	return output;
}

/** patches the smacker_handle_ptr to v5 */
smk_output_stream_t* smk_patch(const uint8_t* buffer, uint32_t size)	//	win32 only
{
	smacker_handle_ptr s = NULL;

	/*smk_assert(buffer);*/

	/* set up the read union for Memory mode */
	uint8_t* fp = (unsigned char*)buffer;

	/* Temporary variables */
	long temp_l;
	unsigned long temp_u = 0l;

	uint64_t out_size = 0;
	uint64_t chunk_1_end_offset = 0;
	uint64_t chunk_2_start_offset = 0;

	/* r is used by macros above for return code */
	char r;
	unsigned char buf[4] = { '\0' };

	/* video hufftrees are stored as a large chunk (bitstream)
		these vars are used to load, then decode them */
	unsigned long tree_size = 0;
	/* a bitstream struct */
	bitstream_t bs;

	/* safe malloc the structure */
	s = memory_allocate(sizeof(smacker_handle_t), MEMORY_DEVELOPMENT);
	memset(s, 0, sizeof(smacker_handle_t));

	/* Check for a valid signature */
	smk_read(buf, 3);
	if (buf[0] != 'S' || buf[1] != 'M' || buf[2] != 'K')
	{
		debug_printf(DEBUG_LOG_ERROR, "libsmacker::smk_open_generic - ERROR: invalid SMKn signature (got: %s)\n", buf);
		goto error;
	}

	/* Read .smacker_handle_ptr file version */
	smk_read(&s->video.v, 1);
	if (s->video.v != '2' && s->video.v != '4' && s->video.v != '5')
	{
		debug_printf(DEBUG_LOG_ERROR, "libsmacker::smk_open_generic - Warning: invalid SMK version %c (expected: 2 or 4)\n", s->video.v);
		/* take a guess */
		if (s->video.v < '4')
			s->video.v = '2';
		else
			s->video.v = '4';
		debug_printf(DEBUG_LOG_ERROR, "\tProcessing will continue as type %c\n", s->video.v);
	}

	if (s->video.v == '5')
	{
		/** already patched */
		return;
	}

	/* width, height, total num frames */
	smk_read_ul(s->video.w);
	smk_read_ul(s->video.h);

	smk_read_ul(s->f);

	/* frames per second calculation */
	smk_read_ul(temp_u);

	/* Video flags follow.
		Ring frame is important to libsmacker.
		Y scale / Y interlace go in the Video flags.
		The user should scale appropriately. */
	smk_read_ul(temp_u);
	if (temp_u & 0x01)
	{
		s->ring_frame = 1;
	}
	if (temp_u & 0x02)
	{
		s->video.y_scale_mode = SMK_FLAG_Y_DOUBLE;
	}
	if (temp_u & 0x04)
	{
		if (s->video.y_scale_mode == SMK_FLAG_Y_DOUBLE)
		{
			debug_printf(DEBUG_LOG_ERROR, "libsmacker::smk_open_generic - Warning: SMK file specifies both Y-Double AND Y-Interlace.\n");
		}
		s->video.y_scale_mode = SMK_FLAG_Y_INTERLACE;
	}

	/* Max buffer size for each audio track - used to pre-allocate buffers */
	for (temp_l = 0; temp_l < 7; temp_l++)
	{
		smk_read_ul(s->audio[temp_l].max_buffer);
	}

	/* Read size of "hufftree chunk" - save for later. */
	smk_read_ul(tree_size);

	/* "unpacked" sizes of each huff tree - we don't use
		but calling application might. */
	for (temp_l = 0; temp_l < 4; temp_l++)
	{
		/*		smk_read_ul(s->video.tree_size[temp_u]); */
		smk_read_ul(temp_u);
	}

	/* read audio rate data */
	for (temp_l = 0; temp_l < 7; temp_l++)
	{
		smk_read_ul(temp_u);

		// Audio stuff 
	}

	/* Skip over Dummy field */
	smk_read_ul(temp_u);

	/* FrameSizes and Keyframe marker are stored together. */
	s->chunk_size = memory_allocate((s->f + s->ring_frame) * sizeof(unsigned long), MEMORY_DEVELOPMENT);

	for (temp_u = 0; temp_u < (s->f + s->ring_frame); temp_u++)
	{
		smk_read_ul(s->chunk_size[temp_u]);

		/* Set Keyframe */
		/*if (s->chunk_size[temp_u] & 0x01)
		{
			s->keyframe[temp_u] = 1;
		}*/
		/* Bits 1 is used, but the purpose is unknown. */
		s->chunk_size[temp_u] &= 0xFFFFFFFC;
	}

	/* That was easy... Now read FrameTypes! */
	s->frame_type = memory_allocate(s->f + s->ring_frame, MEMORY_DEVELOPMENT);
	for (temp_u = 0; temp_u < (s->f + s->ring_frame); temp_u++)
	{
		smk_read(&s->frame_type[temp_u], 1);
	}

	/* HuffmanTrees
		We know the sizes already: read and assemble into
		something actually parse-able at run-time */
	chunk_1_end_offset = fp - buffer;

	/* set up a Bitstream */
	bitstream_initialize(&bs, fp, tree_size);
	fp += tree_size;

	uint32_t tree_sizes[4];


	/* create some tables */
	for (temp_u = 0; temp_u < 4; temp_u++)
	{
		tree_sizes[temp_u] = 0;
		struct smk_huff16_t* tree;

		tree = smk_huff16_build(&bs);

		smk_huff8_count_nodes(tree->t, &tree_sizes[temp_u]);

		s->video.tree[temp_u] = memory_allocate(sizeof(smk_huff16_v5_t), MEMORY_DEVELOPMENT);

		s->video.tree[temp_u]->cache[0] = tree->cache[0];
		s->video.tree[temp_u]->cache[1] = tree->cache[1];
		s->video.tree[temp_u]->cache[2] = tree->cache[2];

		tree_sizes[temp_u] *= sizeof(smk_huff8_v5_t);

		uint8_t* rom;
		rom = memory_allocate(tree_sizes[temp_u], MEMORY_DEVELOPMENT);

		s->video.tree[temp_u]->t = rom;
		smk_huff8_v5_convert(tree->t, rom);

		smk_huff16_free(tree);
	}

	chunk_2_start_offset = fp - buffer;

	/* Handle the rest of the data.
		For MODE_MEMORY, read the chunks and store */

	s->chunk_data = memory_allocate((s->f + s->ring_frame) * sizeof(unsigned char*), MEMORY_DEVELOPMENT);
	for (temp_u = 0; temp_u < (s->f + s->ring_frame); temp_u++)
	{
		s->chunk_data[temp_u] = fp;
		fp += s->chunk_size[temp_u];
	}

	uint32_t chunk_2_end = fp - buffer;
	uint32_t chunk_2_size = chunk_2_end - chunk_2_start_offset;

	/** write v5 smacker_handle_ptr to out_stream */
	out_size = chunk_1_end_offset;
	for (temp_u = 0; temp_u < 4; temp_u++)
		out_size += tree_sizes[temp_u] + 10;
	out_size += chunk_2_size;
	out_size += 4;	//	for aligment 

	uint8_t* out_data = memory_allocate(out_size, MEMORY_DEVELOPMENT);		//	fix out_size
	uint8_t* out_ptr = out_data;

	smk_output_stream_t* output = memory_allocate(sizeof(smk_output_stream_t), MEMORY_DEVELOPMENT);

	/**	write first chunk */
	memory_copy(out_ptr, buffer, chunk_1_end_offset);		//	this size 
	out_ptr += chunk_1_end_offset;

	out_data[3] = '5';

	/**	write huffman chunk
			cache[3]
			tree_size
			tree_data
	 */

	 //	ensure alignment !!! 
	uint32_t offset = (out_ptr - out_data) + 1;
	uint8_t alignment_shift = offset & 0x03;

	*out_ptr++ = alignment_shift;
	out_ptr += alignment_shift;

	for (temp_u = 0; temp_u < 4; temp_u++)
	{
		memory_copy(out_ptr, &s->video.tree[temp_u]->cache[0], 3 * 2);
		out_ptr += 3 * 2;

		memory_copy(out_ptr, &tree_sizes[temp_u], 4);
		out_ptr += 4;

		memory_copy(out_ptr, s->video.tree[temp_u]->t, tree_sizes[temp_u]);
		out_ptr += tree_sizes[temp_u];
	}

	/**	write final chunk */
	memory_copy(out_ptr, buffer + chunk_2_start_offset, chunk_2_size);
	out_ptr += chunk_2_size;


	output->data_ptr = out_data;
	output->data_size = out_size;

	return output;

error:

	smk_close(s);

	return NULL;
}
