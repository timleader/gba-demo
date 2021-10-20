/**
	libsmacker - A C library for decoding .smacker_handle_ptr Smacker Video files
	Copyright (C) 2012-2017 Greg Kennedy

	See smacker.h for more information.

	smacker.c
		Main implementation file of libsmacker.
		Open, close, query, render, advance and seek an smacker_handle_ptr
*/

#include "smacker.h"

#include "common/memory.h"
#include "common/debug/debug.h"
#include "common/utils/bitstream.h"

/* GLOBALS */
/* tree processing order */
#define SMK_TREE_MMAP	0
#define SMK_TREE_MCLR	1
#define SMK_TREE_FULL	2
#define SMK_TREE_TYPE	3


/* A memory_copy wrapper: consumes N bytes, or returns -1
	on failure (when size too low) */
char smk_read_memory(void* buf, const unsigned long size, uint8_t** p, unsigned long* p_size)
{
	if (size > *p_size)
	{
		debug_printf(DEBUG_LOG_ERROR,"libsmacker::smk_read_memory(buf,%lu,p,%lu) - ERROR: Short read\n",(unsigned long)size, (unsigned long)*p_size);
		return -1;
	}
	memory_copy(buf, (const void_ptr)*p, size);
	*p += size;
	*p_size -= size;
	return 0;
}


/* PUBLIC FUNCTIONS */
/* open an smacker_handle_ptr (from a generic Source) */
static smacker_handle_ptr smk_open_generic(const uint8_t* fp, unsigned long size, const unsigned char process_mode)
{
	smacker_handle_ptr s = NULL;

	/* Temporary variables */
	long temp_l;
	unsigned long temp_u;

	/* r is used by macros above for return code */
	char r;
	unsigned char buf[4] = {'\0'};

	/* video hufftrees are stored as a large chunk (bitstream)
		these vars are used to load, then decode them */

	/* safe malloc the structure */
	s = memory_allocate(sizeof (struct smacker_handle_s), MEMORY_EWRAM);
	memory_set(s, sizeof(struct smacker_handle_s), 0);

	/* Check for a valid signature */
	smk_read(buf,3);
	debug_assert(buf[0] == 'S' && buf[1] == 'M' && buf[2] == 'K', "libsmacker::smk_open_generic - ERROR: invalid SMKn signature");

	/* Read .smacker_handle_ptr file version */
	smk_read(&s->video.v,1);
	debug_assert(s->video.v == '5', "libsmacker::smk_open_generic - Warning: invalid SMK version (expected: 5)");

	/* width, height, total num frames */
	smk_read_ul(s->video.w);
	smk_read_ul(s->video.h);

	smk_read_ul(s->f);

	/* frames per second calculation */
	smk_read_ul(temp_u);
	temp_l = (int)temp_u;
	if (temp_l > 0)
	{
		/* millisec per frame */
		s->usf = temp_l * 1000;
	}
	else if (temp_l < 0)
	{
		/* 10 microsec per frame */
		s->usf = temp_l * -10;
	}
	else
	{
		/* defaults to 10 usf (= 100000 microseconds) */
		s->usf = 100000;
	}

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
	for (temp_l = 0; temp_l < 7; temp_l ++)
	{
		smk_read_ul(s->audio[temp_l].max_buffer);
	}

	/* Read size of "hufftree chunk" - save for later. */
	smk_read_ul(temp_u);			//	is this the max tree size ??? 

	/* "unpacked" sizes of each huff tree - we don't use
		but calling application might. */
	for (temp_l = 0; temp_l < 4; temp_l ++)
	{
/*		smk_read_ul(s->video.tree_size[temp_u]); */
		smk_read_ul(temp_u);
	}

	/* read audio rate data */
	for (temp_l = 0; temp_l < 7; temp_l ++)
	{
		s->audio[temp_l].buffer.data = NULL;

		smk_read_ul(temp_u);
		if (temp_u & 0x40000000)
		{
			/* Audio track specifies "exists" flag, malloc structure and copy components. */
			s->audio[temp_l].exists = 1;

			/* and for all audio tracks */ 
			//	doubling the max_buffer as first read seems to be double the size 

			ringbuffer_new(&s->audio[temp_l].buffer, 17204 << 1, MEMORY_EWRAM);		//	calculate from something better ??? thought a fixed size buffer might be usefull...

			if (temp_u & 0x80000000)
			{
				s->audio[temp_l].compress = 1;
			}
			s->audio[temp_l].bitdepth = ((temp_u & 0x20000000) ? 16 : 8);
			s->audio[temp_l].channels = ((temp_u & 0x10000000) ? 2 : 1);

			debug_assert(!(temp_u & 0x0c000000), "libsmacker::smk_open_generic - Warning: audio track is compressed with Bink (perceptual) Audio Codec: this is currently unsupported by libsmacker");

			/* Bits 25 & 24 are unused. */
			s->audio[temp_l].rate = (temp_u & 0x00FFFFFF);
		}
	}

	/* Skip over Dummy field */
	smk_read_ul(temp_u);

	/* FrameSizes and Keyframe marker are stored together. */
	s->chunk_size = (unsigned long*)fp;
	fp += (s->f + s->ring_frame) * sizeof(unsigned long);

	/* That was easy... Now read FrameTypes! */
	s->frame_type = (uint8_t*)fp;
	fp += (s->f + s->ring_frame) * sizeof(uint8_t);

	{
		uint8_t aligment_padding = 0;	
		smk_read(&aligment_padding, 1);		//	aligment_padding is reading wrong 
		fp += aligment_padding;

		uint32_t tree_size = 0;

		/* create some tables to directly reference the gamepak rom memory */
		for (temp_u = 0; temp_u < 3; temp_u++)
		{
			s->video.tree[temp_u] = memory_allocate(sizeof(smk_huff16_v5_t), MEMORY_EWRAM);

			smk_read(&s->video.tree[temp_u]->cache[0], 3 * sizeof(uint16_t));
			smk_read(&tree_size, sizeof(uint32_t));
			s->video.tree[temp_u]->t = (smk_huff8_v5_t*)fp;

			fp += tree_size;
		}

		/* create this table within the IWRAM for faster access */
		temp_u = 3;
		s->video.tree[temp_u] = memory_allocate(sizeof(smk_huff16_v5_t), MEMORY_EWRAM);

		smk_read(&s->video.tree[temp_u]->cache[0], 3 * sizeof(uint16_t));
		smk_read(&tree_size, sizeof(uint32_t));

		s->video.tree[temp_u]->t = memory_allocate(tree_size, MEMORY_EWRAM);
		memory_copy(s->video.tree[temp_u]->t, (const void_ptr)fp, tree_size);

		fp += tree_size;	
	}

	/* Handle the rest of the data.
		For MODE_MEMORY, read the chunks and store */

	s->chunk_data = memory_allocate((s->f + s->ring_frame) * sizeof(unsigned char*), MEMORY_EWRAM);
	for (temp_u = 0; temp_u < (s->f + s->ring_frame); temp_u ++)
	{
		s->chunk_data[temp_u] = (unsigned char*)fp;
		fp += s->chunk_size[temp_u] & 0xFFFFFFFC;
	}
	
	s->cur_frame = 0;

	return s;
}

/* open an smacker_handle_ptr (from a memory buffer) */
smacker_handle_ptr smk_open_memory(const uint8_t* buffer, uint32_t size)
{
	smacker_handle_ptr s = NULL;

	if (!(s = smk_open_generic(buffer, size, 0)))
	{
		debug_printf(DEBUG_LOG_ERROR,"libsmacker::smk_open_memory(buffer,%lu) - ERROR: Fatal error in smk_open_generic, returning NULL.\n",size);
	}

	return s;
}

/* close out an smacker_handle_ptr file and clean up memory */
void smk_close(smacker_handle_ptr s)
{
	unsigned long u;

	/* free video sub-components */
	{
		if (s->video.tree[3] &&
			s->video.tree[3]->t)
		{
			memory_free(s->video.tree[3]->t);
		}

		for (u = 0; u < 4; u ++)
		{
			if (s->video.tree[u])
				memory_free(s->video.tree[u]);
		}
	}

	/* free audio sub-components */
	for (u=0; u<7; u++)
	{
		if (s->audio[u].buffer.data)
			ringbuffer_delete(&s->audio[u].buffer);
	}
		
	/* mem-mode */
	if (s->chunk_data != NULL)
	{
		memory_free(s->chunk_data);
	}

	memory_free(s);
}

/* tell some info about the file */
int8_t smk_info_all(const smacker_handle_ptr object, unsigned long* frame, unsigned long* frame_count, double* usf)
{
	/* sanity check
	smk_assert(object); */
	if (!frame && !frame_count && !usf) {
		debug_printf(DEBUG_LOG_ERROR, "libsmacker::smk_info_all(object,frame,frame_count,usf) - ERROR: Request for info with all-NULL return references\n");
		return -1;
	}
	if (frame)
		*frame = (object->cur_frame % object->f);

	if (frame_count)
		*frame_count = object->f;

	if (usf)
		*usf = object->usf;

	return 0;
}

int8_t smk_info_video(const smacker_handle_ptr object, unsigned long* w, unsigned long* h, unsigned char* y_scale_mode)
{
	/* sanity check
	smk_assert(object); */
	if (!w && !h && !y_scale_mode)
	{
		debug_printf(DEBUG_LOG_ERROR, "libsmacker::smk_info_all(object,w,h,y_scale_mode) - ERROR: Request for info with all-NULL return references\n");
		return -1;
	}

	if (w)
		*w = object->video.w;

	if (h)
		*h = object->video.h;

	if (y_scale_mode)
		*y_scale_mode = object->video.y_scale_mode;

	return 0;
}

int8_t smk_info_audio(const smacker_handle_ptr object, unsigned char* track_mask, unsigned char channels[7], unsigned char bitdepth[7], unsigned long audio_rate[7])
{
	unsigned char i;

	if (!track_mask && !channels && !bitdepth && !audio_rate)
	{
		debug_printf(DEBUG_LOG_ERROR, "libsmacker::smk_info_audio(object,track_mask,channels,bitdepth,audio_rate) - ERROR: Request for info with all-NULL return references\n");
		return -1;
	}
	if (track_mask)
	{
		*track_mask = ( (object->audio[0].exists) |
			 ( (object->audio[1].exists) << 1 ) |
			 ( (object->audio[2].exists) << 2 ) |
			 ( (object->audio[3].exists) << 3 ) |
			 ( (object->audio[4].exists) << 4 ) |
			 ( (object->audio[5].exists) << 5 ) |
			 ( (object->audio[6].exists) << 6 ) );
	}
	if (channels)
	{
		for (i = 0; i < 7; i ++)
		{
			channels[i] = object->audio[i].channels;
		}
	}
	if (bitdepth)
	{
		for (i = 0; i < 7; i ++)
		{  
			bitdepth[i] = object->audio[i].bitdepth;
		}
	}
	if (audio_rate)
	{
		for (i = 0; i < 7; i ++)
		{
			audio_rate[i] = object->audio[i].rate;
		}
	}
	return 0;
}

/* Enable-disable switches */
int8_t smk_enable_all(smacker_handle_ptr object, const unsigned char mask)
{
	unsigned char i;

	/* set video-enable */
	object->video.enable = (mask & 0x80);

	for (i = 0; i < 7; i ++)
	{
		if (object->audio[i].exists)
		{
			object->audio[i].enable = (mask & (1 << i));
		}
	}

	return 0;
}

int8_t smk_enable_video(smacker_handle_ptr object, const unsigned char enable)
{
	object->video.enable = enable;
	return 0;
}

int8_t smk_enable_audio(smacker_handle_ptr object, const unsigned char track, const unsigned char enable)
{
	object->audio[track].enable = enable;
	return 0;
}

const uint16_t* smk_get_palette(const smacker_handle_ptr object)
{
	return &object->video.palette[0];
}

const ringbuffer_t* smk_get_audio(const smacker_handle_ptr object, const unsigned char t)
{
	return &object->audio[t].buffer;		
}

/* Decompresses a palette-frame. */
static char smk_render_palette(struct smk_video_t* s, uint8_t* p, unsigned long size)	//	p -> uint8_t*
{
	/* Index into palette */
	uint16_t i = 0;
	/* Helper variables */
	uint16_t count, src;

	uint8_t r, g, b;

	static uint16_t oldPalette[256];	

	/* sanity check
	smk_assert(s);
	smk_assert(p); */

	// Copy palette to old palette
	memory_copy(oldPalette, s->palette, 256 * sizeof(uint16_t));

	/* Loop until palette is complete, or we are out of bytes to process */
	while ( (i < 256) && (size > 0) )
	{
		if ((*p) & 0x80)
		{
			/* 0x80: Skip block
				(preserve C+1 palette entries from previous palette) */
			count = ((*p) & 0x7F) + 1;
			p ++; size --;

			/* check for overflow condition */
			if (i + count > 256)
			{
				debug_printf(DEBUG_LOG_ERROR,"libsmacker::palette_render(s,p,size) - ERROR: overflow, 0x80 attempt to skip %d entries from %d\n",count,i);
				goto error;
			}

			/* finally: advance the index. */
			i += count;
		}
		else if ((*p) & 0x40)
		{
			/* 0x40: Color-shift block
				Copy (c + 1) color entries of the previous palette,
				starting from entry (s),
				to the next entries of the new palette. */
			if (size < 2)
			{
				debug_printf(DEBUG_LOG_ERROR, "libsmacker::palette_render(s,p,size) - ERROR: 0x40 ran out of bytes for copy\n");
				goto error;
			}

			/* pick "count" items to copy */
			count = ((*p) & 0x3F) + 1;
			p ++; size --;

			/* start offset of old palette */
			src = *p;
			p ++; size --;

			/* overflow: see if we write/read beyond 256colors, or overwrite own palette */
			if (i + count > 256 || src + count > 256 ||
				(src < i && src + count > i) )
			{
				debug_printf(DEBUG_LOG_ERROR,"libsmacker::palette_render(s,p,size) - ERROR: overflow, 0x40 attempt to copy %d entries from %d to %d\n",count,src,i);
				goto error;
			}

			/* OK!  Copy the color-palette entries. */
			memory_copy(&s->palette[i],&oldPalette[src],count * 2);

			i += count;
		}
		else
		{
			/* 0x00: Set Color block
				Direct-set the next 3 bytes for palette index */
			if (size < 3)
			{
				debug_printf(DEBUG_LOG_ERROR,"libsmacker::palette_render - ERROR: 0x3F ran out of bytes for copy, size=%lu\n", size);
				goto error;
			}

			/*	should do a if (*p > 0x3F) for each component */
			r = ((*p) & 0x3F) >> 1;
			p++; size--;

			g = ((*p) & 0x3F) >> 1;
			p++; size--;

			b = ((*p) & 0x3F) >> 1;
			p++; size--;

			s->palette[i] = (b << 10) | (g << 5) | (r);

			i ++;
		}
	}

	if (i < 256)
	{
		debug_printf(DEBUG_LOG_ERROR,"libsmacker::palette_render - ERROR: did not completely fill palette (idx=%u)\n",i);
		goto error;
	}

	return 0;

error:
	/* Error, return -1
		The new palette probably has errors but is preferrable to a black screen */
	return -1;
}

#define RGB8(r,g,b)	( (((b)>>3)<<10) | (((g)>>3)<<5) | ((r)>>3) )

IWRAM_CODE static char smk_render_video(struct smk_video_t* s, const uint16_t* frame, uint16_t pitch, unsigned char* p, unsigned int size)
{
	uint16_t* t = (uint16_t*)frame;		//	should write in blocks of int16 or maybe even int32
	uint8_t s1,s2;
	uint16_t temp1, temp2;
	uint16_t i,j,k, row, half_col,skip;
	uint16_t width = s->w >> 1;

	/* used for video decoding */
	bitstream_t bs;

	/* results from a tree lookup */
	uint16_t unpack;

	/* unpack, broken into pieces */
	uint8_t type;
	uint8_t blocklen;
	uint16_t typedata;
	char bit;

	const uint16_t sizetable[64] = 
	{
		1,	 2,	3,	4,	5,	6,	7,	8,
		9,	10,	11,	12,	13,	14,	15,	16,
		17,	18,	19,	20,	21,	22,	23,	24,
		25,	26,	27,	28,	29,	30,	31,	32,
		33,	34,	35,	36,	37,	38,	39,	40,
		41,	42,	43,	44,	45,	46,	47,	48,
		49,	50,	51,	52,	53,	54,	55,	56,
		57,	58,	59,	128,	256,	512,	1024,	2048
	};

	row = 0;
	half_col = 0;

	/* Set up a bitstream for video unpacking */
	/* We could check the return code but it will only fail if p is null and we already verified that. */

	bitstream_initialize(&bs, (const void_ptr)p, size);

	/* Reset the cache on all bigtrees */
	smk_huff16_v5_reset(s->tree[0]);
	smk_huff16_v5_reset(s->tree[1]);
	smk_huff16_v5_reset(s->tree[2]);
	smk_huff16_v5_reset(s->tree[3]);

	while (row < s->h)
	{
		unpack = smk_huff16_v5_lookup(&bs, s->tree[SMK_TREE_TYPE]);		//	decompress this ahead of time???
		
		type = (unpack & 0x0003);
		blocklen = ((unpack & 0x00FC) >> 2);
		typedata = ((unpack & 0xFF00) >> 8);
		typedata |= typedata << 8;

		/* support for v4 full-blocks */
		if (type == 1)
		{
			bit = bitstream_read_1(&bs);
			if (bit)
			{
				type = 4;
			} 
			else 
			{
				bit = bitstream_read_1(&bs);
				if (bit)
				{
					type = 5;
				}
			}
		}

		for (j = 0; (j < sizetable[blocklen]) && (row < s->h); j ++)	//	move loop into each switch ... case ... 
		{
			skip = (row * pitch) + half_col;

			switch (type)	
			{
				case 0:	
					unpack = smk_huff16_v5_lookup(&bs, s->tree[SMK_TREE_MCLR]);
					s1 = (unpack & 0xFF00) >> 8;
					s2 = (unpack & 0x00FF);

					unpack = smk_huff16_v5_lookup(&bs, s->tree[SMK_TREE_MMAP]);

					temp1 = 0x01;
					for (k = 0; k < 4; k ++)
					{
						for (i = 0; i < 2; i ++)
						{
							if (unpack & temp1)
							{
								temp2 = s1;
							}
							else
							{
								temp2 = s2;
							}
							temp1 = temp1 << 1;

							if (unpack & temp1)
							{
								temp2 |= s1 << 8;
							}
							else
							{
								temp2 |= s2 << 8;
							}
							temp1 = temp1 << 1;

							t[skip + i] = temp2;
						}
						skip += pitch;
					}
					break;

				case 1: /* FULL BLOCK */
					for (k = 0; k < 4; k ++)
					{
						unpack = smk_huff16_v5_lookup(&bs, s->tree[SMK_TREE_FULL]);
						t[skip + 1] = unpack;

						unpack = smk_huff16_v5_lookup(&bs, s->tree[SMK_TREE_FULL]);
						t[skip] = unpack;

						skip += pitch;
					}
					break;
				case 2: /* VOID BLOCK */		
					break;
				case 3: /* SOLID BLOCK */	
					t[skip] = typedata;	
					t[skip + 1] = typedata;
					skip += pitch;

					t[skip] = typedata;
					t[skip + 1] = typedata;
					skip += pitch;

					t[skip] = typedata;
					t[skip + 1] = typedata;
					skip += pitch;

					t[skip] = typedata;
					t[skip + 1] = typedata;
					break;
				case 4: /* V4 DOUBLE BLOCK */
					for (k = 0; k < 2; k ++)
					{
						unpack = smk_huff16_v5_lookup(&bs, s->tree[SMK_TREE_FULL]);
						for (i = 0; i < 2; i ++)
						{
							t[skip] = ((unpack & 0x00FF) << 8) | (unpack & 0x00FF);
							t[skip + 1] = ((unpack & 0xFF00) >> 8) | (unpack & 0xFF00);

							skip += pitch;
						}
					}
					break;
				case 5: /* V4 HALF BLOCK */
					for (k = 0; k < 2; k ++)		
					{
						unpack = smk_huff16_v5_lookup(&bs, s->tree[SMK_TREE_FULL]);
						t[skip + 1] = unpack;
						t[skip + pitch + 1] = unpack;

						unpack = smk_huff16_v5_lookup(&bs, s->tree[SMK_TREE_FULL]);
						t[skip] = unpack;
						t[skip + pitch] = unpack;

						skip += (pitch << 1);
					}
					break;
			}
			half_col += 2;
			if (half_col >= width)
			{
				half_col = 0;
				row += 4;
			}
		}
	}

	return 0;
}

/* Decompress audio track i. */
static char smk_render_audio(struct smk_audio_t* s, unsigned char* p, unsigned long size)
{
	uint32_t j,k;
	ringbuffer_t* t = &s->buffer;
	bitstream_t bs;

	if (!s->compress)
	{
		/* Raw PCM data, update buffer size and malloc */
		//ringbuffer_copy_to(t, p, size);		//	fix this !!! 
	}
	else if (s->compress == 1)		//	this isn't very compressed :-(
	{
		//	let's generalize this as my adpcm implementation ...

		uint8_t bit;
		int8_t unpack;
		int8_t sample0, sample1;

		/* used for audio decoding */
		struct smk_huff8_t* aud_tree[2] = { NULL, NULL };

		/* SMACKER DPCM compression */

		debug_assert(size >= 4, "libsmacker::smk_render_audio() - ERROR: need 4 bytes to get unpacked output buffer size");

		///	this is stupidly big on the first frame ... ??? 

		/* chunk is compressed (huff-compressed dpcm), retrieve unpacked buffer size */
		uint32_t chunk_size = 
			((uint32_t) p[3] << 24) |
			((uint32_t) p[2] << 16) |
			((uint32_t) p[1] << 8) |
			((uint32_t) p[0]);

		p += 4;
		size -= 4;

		/* Compressed audio: must unpack here */
		/*  Set up a bitstream */
		bitstream_initialize(&bs, p, size);

		bit = bitstream_read_1(&bs);

		debug_assert(bit, "libsmacker::smk_render_audio - ERROR: initial get_bit returned 0");

		bit = bitstream_read_1(&bs);
		debug_assert(s->channels = (bit == 1 ? 2 : 1), "libsmacker::smk_render - ERROR: mono/stereo mismatch");

		bit = bitstream_read_1(&bs);
		debug_assert(s->bitdepth == (bit == 1 ? 16 : 8), "libsmacker::smk_render - ERROR: 8-/16-bit mismatch");		

		/* build the trees */
		aud_tree[0] = smk_huff8_build(&bs);	
		j = 1;
		k = 1;

		if (s->channels == 2)
		{
			aud_tree[1] = smk_huff8_build(&bs);
			j = 2;
			k = 2;
		}

		/* read initial sound levels */
		sample0 = bitstream_read_8(&bs);
		sample0 -= 128;		//	uint8_t to int8_t
		ringbuffer_write(t, sample0);

		if (s->channels == 2)
		{
			sample1 = bitstream_read_8(&bs);
			ringbuffer_write(t, sample1);
		}

		/* All set: let's read some DATA! */
		while (k < chunk_size)
		{
			unpack = smk_huff8_lookup(&bs, aud_tree[0]);
			if (unpack < 0)
			{
				unpack = unpack;
			}
			sample0 += unpack;		//	how does this come out negative 
			ringbuffer_write(t, (int8_t)sample0);

			k++;

			if (s->channels == 2)
			{
				unpack = smk_huff8_lookup(&bs, aud_tree[1]);
				sample1 += unpack;
				ringbuffer_write(t, (int8_t)sample1);

				k++;
			}
		}

		/* All done with the trees, free them. */
		for (j = 0; j < 2; j ++)
		{
			if (aud_tree[j])
			{
				smk_huff8_free(aud_tree[j]);
			}
		}
	}

	return 0;
}

/* "Renders" (unpacks) the frame at cur_frame
	Preps all the image and audio pointers */
	/* Convenience call-out for recursive bigtree lookup function */
/*IWRAM_CODE*/ static char smk_render(smacker_handle_ptr s, const uint16_t* frame, uint16_t pitch)
{
	unsigned long i,size;
	unsigned char* buffer = NULL,* p,track;

	/* Retrieve current chunk_size for this frame. */
	if (!(i = (s->chunk_size[s->cur_frame] & 0xFFFFFFFC)))
	{
		debug_printf(DEBUG_LOG_ERROR,"libsmacker::smk_render(s) - Warning: frame %lu: chunk_size is 0.\n",s->cur_frame);
		return -1;
	}

	/* Just point buffer at the right place */
	if (!s->chunk_data[s->cur_frame])
	{
		debug_printf(DEBUG_LOG_ERROR,"libsmacker::smk_render(s) - ERROR: frame %lu: memory chunk is a NULL pointer.\n",s->cur_frame);
		return -1;
	}
	buffer = s->chunk_data[s->cur_frame];

	p = buffer;

	/* Palette record first */
	if (s->frame_type[s->cur_frame] & 0x01)
	{
		/* need at least 1 byte to process */
		if (!i)
		{
			debug_printf(DEBUG_LOG_ERROR,"libsmacker::smk_render(s) - ERROR: frame %lu: insufficient data for a palette rec.\n",s->cur_frame);
			return -1;
		}

		/* Byte 1 in block, times 4, tells how many
			subsequent bytes are present */
		size = 4 * (*p);

		/* If video rendering active, kick this off for decode. */
		if (s->video.enable)
		{
			smk_render_palette(&(s->video),p + 1,size - 1);
		}
		p += size;
		i -= size;
	}

	/* Unpack audio chunks */
	for (track = 0; track < 7; track ++)
	{
		if (s->frame_type[s->cur_frame] & (0x02 << track))
		{
			/* need at least 4 byte to process */
			if (i < 4)
			{
				debug_printf(DEBUG_LOG_ERROR,"libsmacker::smk_render(s) - ERROR: frame %lu: insufficient data for audio[%u] rec.\n",s->cur_frame,track);
				return -1;
			}

			/* First 4 bytes in block tell how many
				subsequent bytes are present */
			size = (((unsigned int) p[3] << 24) |
					((unsigned int) p[2] << 16) |
					((unsigned int) p[1] << 8) |
					((unsigned int) p[0]));

			/* If audio rendering active, kick this off for decode.*/
			if (s->audio[track].enable)
			{
				smk_render_audio(&s->audio[track], p + 4, size - 4);
			} 
			p += size;
			i -= size;
		}
	}

	/* Unpack video chunk */
	if (s->video.enable)
	{
		smk_render_video(&(s->video), frame, pitch, p, i);
	}

	return 0;
}

/* rewind to first frame and unpack */
int8_t smk_first(smacker_handle_ptr s, const uint8_t* output_framebuffer, const uint16_t output_pitch)
{
	/*smk_assert(s);*/

	s->cur_frame = 0;
	if ( smk_render(s, (const uint16_t*)output_framebuffer, output_pitch >> 1) < 0)
	{
		debug_printf(DEBUG_LOG_ERROR,"libsmacker::smk_first(s) - Warning: frame %lu: smk_render returned errors.\n",s->cur_frame);

		return -1;
	}

	if (s->f == 1)
		return SMK_LAST;

	return SMK_MORE;
}

/* advance to next frame */
int8_t smk_next(smacker_handle_ptr s, const uint8_t* output_framebuffer, const uint16_t output_pitch)
{
	debug_assert(s != NULL, "smk::next smacker_handle == NULL");

	if (s->cur_frame + 1 < (s->f + s->ring_frame))
	{
		s->cur_frame ++;
		if ( smk_render(s, (const uint16_t*)output_framebuffer, output_pitch >> 1) < 0)
		{
			debug_printf(DEBUG_LOG_ERROR,"libsmacker::smk_next(s) - Warning: frame %lu: smk_render returned errors.\n",s->cur_frame);
			return -1;
		}
		if (s->cur_frame + 1 == (s->f + s->ring_frame))
		{
			return SMK_LAST;
		}
		return SMK_MORE;
	}
	else if (s->ring_frame)
	{
		s->cur_frame = 1;
		if ( smk_render(s, (const uint16_t*)output_framebuffer, output_pitch >> 1) < 0)
		{
			debug_printf(DEBUG_LOG_ERROR,"libsmacker::smk_next(s) - Warning: frame %lu: smk_render returned errors.\n",s->cur_frame);
			return -1;
		}
		if (s->cur_frame + 1 == (s->f + s->ring_frame))
		{
			return SMK_LAST;
		}
		return SMK_MORE;
	}
	return SMK_DONE;
}

/* seek to a keyframe in an smacker_handle_ptr */
int8_t smk_seek_keyframe(smacker_handle_ptr s, const uint8_t* output_framebuffer, const uint16_t output_pitch, unsigned long f)
{
	debug_assert(s != NULL, "smk::seek_keyframe smacker_handle == NULL");

	/* rewind (or fast forward!) exactly to f */
	s->cur_frame = f;

	/* roll back to previous keyframe in stream, or 0 if no keyframes exist */
	while (s->cur_frame > 0 && !(s->chunk_size[s->cur_frame] & 0x01))
	{
		s->cur_frame --;
	}

	/* render the frame: we're ready */
	if (smk_render(s, (const uint16_t*)output_framebuffer, output_pitch >> 1) < 0)
	{
		debug_printf(DEBUG_LOG_ERROR,"libsmacker::smk_seek_keyframe(s,%lu) - Warning: frame %lu: smk_render returned errors.\n",f,s->cur_frame);
		return -1;
	}

	return 0;
}
