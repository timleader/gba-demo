
#ifndef SMACKER_H
#define SMACKER_H

#include "common/types.h"
#include "common/utils/ringbuffer.h"

/* data structures */
#include "smk_hufftree.h"
#include "smk_hufftree_v5.h"

/** forward-declaration for an struct */

typedef struct smk_output_stream_s
{
	uint8_t* data_ptr;
	uint32_t data_size;
} smk_output_stream_t;

/** a few defines as return codes from smk_next() */
#define SMK_DONE	0x00
#define SMK_MORE	0x01
#define SMK_LAST	0x02
#define SMK_ERROR	-1

/** Y-scale meanings */
#define	SMK_FLAG_Y_NONE	0x00
#define	SMK_FLAG_Y_INTERLACE	0x01
#define	SMK_FLAG_Y_DOUBLE	0x02

/** track mask and enable bits */
#define	SMK_AUDIO_TRACK_0	0x01
#define	SMK_AUDIO_TRACK_1	0x02
#define	SMK_AUDIO_TRACK_2	0x04
#define	SMK_AUDIO_TRACK_3	0x08
#define	SMK_AUDIO_TRACK_4	0x10
#define	SMK_AUDIO_TRACK_5	0x20
#define	SMK_AUDIO_TRACK_6	0x40

#define	SMK_VIDEO_TRACK		0x80

/* SMACKER DATA STRUCTURES */
typedef struct smacker_handle_s
{
	/* microsec per frame - stored as a double to handle scaling
		(large positive millisec / frame values may overflow a ul) */
	double	usf;		//	double ??? 

	/* total frames */
	unsigned long	f;
	/* does file have a ring frame? (in other words, does file loop?) */
	unsigned char	ring_frame;

	/* Index of current frame */
	unsigned long	cur_frame;


	/* in-memory mode: unprocessed chunks */
	unsigned char** chunk_data;

	/* shared array of "chunk sizes"*/
	unsigned long* chunk_size;

	/* Holds per-frame type mask (e.g. 'audio track 3, 2, and palette swap') */
	uint8_t* frame_type;

	/* video and audio structures */
	/* Video data type: enable/disable decode switch,
		video info and flags,
		pointer to last-decoded-palette */
	struct smk_video_t
	{
		/* enable/disable decode switch */
		unsigned char enable;

		/* video info */
		unsigned long	w;
		unsigned long	h;
		/* Y scale mode (constants defined in smacker.h)
			0: unscaled
			1: doubled
			2: interlaced */
		unsigned char	y_scale_mode;

		/* version ('2' or '4') */
		uint8_t	v;

		/* Huffman trees */
		/* unsigned long tree_size[4]; */
		smk_huff16_v5_t* tree[4];		//	do away with huff16_t and have huff8_t amd cache[3] separate

		/* Palette data type: pointer to last-decoded-palette */
		uint16_t palette[256];

	} video;

	/* audio structure */
	struct smk_audio_t
	{
		/* set if track exists in file */
		bool8_t exists;

		/* enable/disable switch (per track) */
		bool8_t enable;

		/* Info */
		uint8_t	channels;
		uint8_t	bitdepth;

		unsigned long	rate;
		long	max_buffer;

		/* compression type
			0: raw PCM
			1: SMK DPCM
			2: Bink (Perceptual), unsupported */
		unsigned char	compress;

		/* pointer to last-decoded-audio-buffer */
		ringbuffer_t buffer;				//	turn into ring-buffer with x frames worth of data ...
											//		will need to handle audio callback, knowing where we are currently read / write positions are.. 

	} audio[7];			//	hardware will probably only support 4 channels so limit to first 4 channel, get rid of the rest 

} smacker_handle_t;

typedef smacker_handle_t* smacker_handle_ptr;

char smk_read_memory(void* buf, const unsigned long size, uint8_t** p, unsigned long* p_size);

/* Helper functions to do the reading, plus
	byteswap from LE to host order */
	/* read n bytes from (source) into ret */
#define smk_read(ret,n) \
{ \
	r = (smk_read_memory(ret,n,(uint8_t**)&fp,&size)); \
	debug_assert(r >= 0, "libsmacker::smk_read(...) - Errors encountered on read"); \
}

/* Calls smk_read, but returns a ul */
#define smk_read_ul(p) \
{ \
	smk_read(buf,4); \
	p = ((unsigned long) buf[3] << 24) | \
		((unsigned long) buf[2] << 16) | \
		((unsigned long) buf[1] << 8) | \
		((unsigned long) buf[0]); \
}


/* OPEN OPERATIONS */
/** read an smacker_handle_ptr (from a memory buffer) */
smacker_handle_ptr smk_open_memory(const uint8_t* buffer, uint32_t size);

/* CLOSE OPERATIONS */
/** close out an smacker_handle_ptr file and clean up memory */
void smk_close(smacker_handle_ptr object);

/* GET FILE INFO OPERATIONS */
int8_t smk_info_all(const smacker_handle_ptr object, unsigned long* frame, unsigned long* frame_count, double* usf);
int8_t smk_info_video(const smacker_handle_ptr object, unsigned long* w, unsigned long* h, unsigned char* y_scale_mode);
int8_t smk_info_audio(const smacker_handle_ptr object, unsigned char* track_mask, unsigned char channels[7], unsigned char bitdepth[7], unsigned long audio_rate[7]);

/* ENABLE/DISABLE Switches */
int8_t smk_enable_all(smacker_handle_ptr object, unsigned char mask);
int8_t smk_enable_video(smacker_handle_ptr object, unsigned char enable);
int8_t smk_enable_audio(smacker_handle_ptr object, unsigned char track, unsigned char enable);

/** Retrieve palette */
const uint16_t* smk_get_palette(const smacker_handle_ptr object);
/** Retrieve decoded audio chunk, track N */
const ringbuffer_t* smk_get_audio(const smacker_handle_ptr object, const unsigned char t);

/** rewind to first frame and unpack */
int8_t smk_first(smacker_handle_ptr object, const uint8_t* output_framebuffer, const uint16_t output_pitch);
/** advance to next frame and unpack */
int8_t smk_next(smacker_handle_ptr s, const uint8_t* output_framebuffer, const uint16_t output_pitch);
/** seek to first keyframe before/at N in an smacker_handle_ptr */
int8_t smk_seek_keyframe(smacker_handle_ptr object, const uint8_t* output_framebuffer, const uint16_t output_pitch, unsigned long frame_idx);


uint32_t smacker_audio_chunk_uncompressed_size(uint8_t* input, uint32_t size);

void smacker_render_audio_s8_mono(ringbuffer_t* output, uint8_t* input, uint32_t size);

#ifndef __GBA__

/** patches the smacker_handle_ptr to v5 */
smk_output_stream_t* smk_patch(const uint8_t* buffer, uint32_t size);

#endif

#endif
