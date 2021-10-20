
#include "smacker.h"

#include "common/memory.h"
#include "common/debug/debug.h"
#include "common/utils/bitstream.h"
#include "common/utils/profiler.h""


uint32_t smacker_audio_chunk_uncompressed_size(uint8_t* input, uint32_t size)
{
	debug_assert(size >= 4, "libsmacker::smk_render_audio() - ERROR: need 4 bytes to get unpacked output buffer size");

	uint32_t chunk_size =
		((uint32_t)input[3] << 24) |
		((uint32_t)input[2] << 16) |
		((uint32_t)input[1] << 8) |
		((uint32_t)input[0]);

	return chunk_size;
}

IWRAM_CODE void smacker_render_audio_s8_mono(ringbuffer_t* output, uint8_t* input, uint32_t size)
{
	profiler_sample_handle_t profiler_handle;

	smk_huff8_v5_t* aud_tree = NULL;

	int8_t unpack, sample;
	bitstream_t bs;
	uint8_t bit;
	uint32_t k;

	/* SMACKER DPCM compression */
	debug_assert(size >= 4, "libsmacker::smk_render_audio() - ERROR: need 4 bytes to get unpacked output buffer size");

	/* chunk is compressed (huff-compressed dpcm), retrieve unpacked buffer size */
	uint32_t chunk_size =
		((uint32_t)input[3] << 24) |
		((uint32_t)input[2] << 16) |
		((uint32_t)input[1] << 8) |
		((uint32_t)input[0]);

	input += 4;
	size -= 4;

	/* Compressed audio: must unpack here */
	/*  Set up a bitstream */
	bitstream_initialize(&bs, input, size);

	bit = bitstream_read_1(&bs);

	debug_assert(bit, "smacker::smk_render_audio - ERROR: initial get_bit returned 0");

	bit = bitstream_read_1(&bs);
	debug_assert(bit == 0, "smacker::smk_render - ERROR: mono/stereo mismatch");

	bit = bitstream_read_1(&bs);
	debug_assert(bit == 0, "smacker::smk_render - ERROR: 8-/16-bit mismatch");

	profiler_handle = profiler_begin("huff8:b");		//	~8,000 cycles

	/* build the trees */
	aud_tree = smk_huff8_v5_build(&bs);		//	profile tree_build vs tree_lookup 

	profiler_end(profiler_handle);

	/* read initial sound levels */
	sample = bitstream_read_8(&bs);

	ringbuffer_write(output, sample);		// this needs to be inline 
	k = 1;

	profiler_handle = profiler_begin("huff8:l");		//	~330,000 cycles  --- 820 samples 

	//	not sure if this will be possible ...???	maybe ~160,000 cycles might be possible???  => ~20% cpu time    (how the fuck would crossfading be possible with this)

	//	A different DPCM might make sense, 5-bit samples ? with a lut for deltas so we can have a non-linear distribution ... this should be almost as fast as a copy (there is a ldr per sample) 
	//		will compress to 62.5% of original size --> this is about the same as smacker, for some quality loss. 

	//	smacker audio can still be used for video playback, as we should have the cpu time 

	/* All set: let's read some DATA! */
	while (k < chunk_size)		//	convert this loop to assembly or rather this whole function 
	{
		unpack = smk_huff8_v5_lookup(&bs, aud_tree);
		sample += unpack;						
		ringbuffer_write(output, (int8_t)sample);
		k++;
	}

	profiler_end(profiler_handle);

	/* All done with the trees, free them. */
	memory_free(aud_tree);		//	this is failing
}
